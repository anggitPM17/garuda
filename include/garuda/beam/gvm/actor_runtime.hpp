#pragma once
// Garuda GVM: Actor Model Runtime (`actor_runtime.hpp`)
// Mengelola proses actor independen, O(1) ready queue, O(1) unordered_map, fault isolation (kill_actor), dan message passing.
#include "interpreter.hpp"
#include "../core/runtime.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam {

struct GvmActor {
    Pid pid;
    GvmProcessState vm_state;
    std::queue<Term> mailbox;
    bool done = false;
    bool in_ready_queue = false;
    Term return_value;
};

class GvmActorRuntime {
public:
    GvmActorRuntime(std::vector<Instruction> instructions, std::unordered_map<std::string, std::size_t> function_table)
        : instructions_(std::move(instructions)), function_table_(std::move(function_table)) {}

    void set_verbose(bool verbose) { verbose_ = verbose; }

    std::size_t get_total_messages_sent() const { return total_messages_sent_.load(); }
    std::size_t get_total_actors_spawned() const { return total_actors_spawned_.load(); }

    Pid spawn_actor(std::size_t start_ip) {
        total_actors_spawned_++;
        std::lock_guard<std::mutex> lock(mu_);
        Pid new_pid{next_pid_++};
        auto actor = std::make_unique<GvmActor>();
        actor->pid = new_pid;
        actor->vm_state.self_pid = new_pid;
        actor->vm_state.ip = start_ip;
        actor->in_ready_queue = true;
        actors_[new_pid.id] = std::move(actor);
        ready_queue_.push(new_pid.id); // Add directly to O(1) Ready Queue!
        if (verbose_) {
            std::cout << "🐣 [GVM ACTOR SPAWN] Created new Actor Process PID: " << to_string(Term(new_pid)) << " at IP: " << start_ip << std::endl;
        }
        return new_pid;
    }

    void send_message(Pid target_pid, Term message) {
        total_messages_sent_++;
        std::lock_guard<std::mutex> lock(mu_);
        auto it = actors_.find(target_pid.id);
        if (it != actors_.end()) {
            it->second->mailbox.push(message);
            if (!it->second->done && !it->second->in_ready_queue) {
                it->second->in_ready_queue = true;
                ready_queue_.push(target_pid.id); // Wake up actor into O(1) Ready Queue!
            }
            if (verbose_) {
                std::cout << "✉️ [GVM ACTOR SEND] Message sent to PID " << to_string(Term(target_pid)) << ": " << to_string(message) << std::endl;
            }
        }
    }

    std::optional<Term> pop_mailbox(Pid self_pid) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = actors_.find(self_pid.id);
        if (it != actors_.end() && !it->second->mailbox.empty()) {
            Term msg = it->second->mailbox.front();
            it->second->mailbox.pop();
            return msg;
        }
        return std::nullopt;
    }

    void kill_actor(Pid pid) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = actors_.find(pid.id);
        if (it != actors_.end()) {
            it->second->done = true;
            std::queue<Term> empty;
            std::swap(it->second->mailbox, empty); // Reclaim mailbox memory immediately!
            if (verbose_) {
                std::cout << "☠️ [GVM ACTOR KILL] Process PID " << to_string(Term(pid)) << " killed & mailbox reclaimed." << std::endl;
            }
        }
    }

    bool is_actor_alive(Pid pid) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = actors_.find(pid.id);
        return it != actors_.end() && !it->second->done;
    }

    bool is_actor_done(Pid pid) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = actors_.find(pid.id);
        if (it != actors_.end()) return it->second->done;
        return true;
    }

    std::size_t active_actor_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return actors_.size();
    }

    std::size_t alive_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (const auto& [pid_id, actor] : actors_) {
            if (!actor->done) count++;
        }
        return count;
    }

    std::size_t estimated_memory_bytes() const {
        std::lock_guard<std::mutex> lock(mu_);
        return actors_.size() * sizeof(GvmActor);
    }

    std::optional<Term> run_until_done(const std::string& entry_function, std::size_t max_iterations = 10000000) {
        auto it = function_table_.find(entry_function);
        if (it == function_table_.end()) {
            std::cout << "⚠️ [GVM ACTOR RUNTIME] Entry function '" << entry_function << "' not found!" << std::endl;
            return std::nullopt;
        }

        Pid root_pid = spawn_actor(it->second);
        Runtime dummy_rt;
        Pid dummy_pid = dummy_rt.spawn([](Process&) { return Status::Done; });
        Process* proc_ptr = dummy_rt.find(dummy_pid);

        GvmInterpreter interpreter(instructions_);
        interpreter.set_callbacks(
            [this](std::size_t target_ip) -> Pid {
                return spawn_actor(target_ip);
            },
            [this](Pid target, Term msg) {
                send_message(target, msg);
            },
            [this](Pid self) -> std::optional<Term> {
                return pop_mailbox(self);
            }
        );

        std::size_t loop_count = 0;

        while (loop_count++ < max_iterations) {
            std::uint64_t pid_id = 0;
            {
                std::lock_guard<std::mutex> lock(mu_);
                if (ready_queue_.empty()) break;
                pid_id = ready_queue_.front();
                ready_queue_.pop();
            }

            GvmActor* actor = nullptr;
            {
                std::lock_guard<std::mutex> lock(mu_);
                auto a_it = actors_.find(pid_id);
                if (a_it != actors_.end() && !a_it->second->done) {
                    actor = a_it->second.get();
                    actor->in_ready_queue = false; // Popped from ready queue!
                }
            }

            if (!actor) continue;

            Status status = interpreter.execute_slice(*proc_ptr, actor->vm_state);
            if (status == Status::Done) {
                actor->done = true;
                actor->return_value = actor->vm_state.x_regs[0];

                if (pid_id == root_pid.id) {
                    return actor->return_value; // Root process finished! Return immediately!
                } else {
                    std::lock_guard<std::mutex> lock(mu_);
                    actors_.erase(pid_id);
                }
            } else if (status == Status::TailCall) {
                actor->vm_state.ip = actor->vm_state.tail_call_target;
                std::lock_guard<std::mutex> lock(mu_);
                if (!actor->in_ready_queue) {
                    actor->in_ready_queue = true;
                    ready_queue_.push(pid_id); // Re-queue trampolined tail call!
                }
            } else if (status == Status::Continue) {
                std::lock_guard<std::mutex> lock(mu_);
                if (!actor->in_ready_queue) {
                    actor->in_ready_queue = true;
                    ready_queue_.push(pid_id); // Re-queue runnable actor to end of O(1) Ready Queue!
                }
            } else if (status == Status::Wait) {
                // Actor is waiting for mailbox message. Do not re-queue until message arrives!
            }
        }

        std::lock_guard<std::mutex> lock(mu_);
        auto root_it = actors_.find(root_pid.id);
        if (root_it != actors_.end() && root_it->second->done) {
            return root_it->second->return_value;
        }
        return std::nullopt;
    }

private:
    mutable std::mutex mu_;
    std::atomic<std::size_t> total_messages_sent_{0};
    std::atomic<std::size_t> total_actors_spawned_{0};
    std::uint64_t next_pid_ = 1;
    bool verbose_ = true;
    std::vector<Instruction> instructions_;
    std::unordered_map<std::string, std::size_t> function_table_;
    std::unordered_map<std::uint64_t, std::unique_ptr<GvmActor>> actors_; // Rehash-safe with unique_ptr!
    std::vector<std::unique_ptr<GvmActor>> pool_;                         // Slab Memory Pool!
    std::queue<std::uint64_t> ready_queue_;                               // O(1) Ready Queue!
};

} // namespace garuda::beam
