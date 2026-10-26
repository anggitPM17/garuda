#pragma once
// Garuda BEAM-core: proses ringan, mailbox + selective receive, link/monitor/exit signal,
// scheduler berbasis reduction (preemptive-kooperatif), registry nama, timer.
#include "term.hpp"
#include "timer_wheel.hpp"

#include <cstdint>
#include <cstdio>
#include <exception>
#include <functional>
#include <list>
#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam {

class Runtime;
class Process;

// Continue = masih ada kerja (yield); Wait = blok sampai ada pesan baru;
// Done = exit normal; Exit = exit dengan alasan yang di-set lewat Process::stop().
enum class Status { Continue, Wait, Done, Exit, TailCall };
enum class ProcState { Runnable, Running, Waiting, Exited };

struct Message { Pid from; Term body; };
using Body = std::function<Status(Process&)>;
using MessagePred = std::function<bool(const Message&)>;

struct SpawnOptions {
    bool link = false;
    bool trap_exit = false;
    std::string name;
    Pid parent{};
};

struct RuntimeStats {
    std::uint64_t spawned = 0, exited = 0, messages = 0, reductions = 0, slices = 0, preemptions = 0;
    std::uint64_t dropped = 0;    // pesan dibuang karena mailbox penuh
    std::uint64_t rejected = 0;   // spawn ditolak karena batas proses
    std::uint64_t remote = 0;     // pesan lintas shard
    std::uint64_t crashes = 0;    // exit dengan alasan abnormal
};

// Hook observabilitas (log/metrics). Dipanggil sinkron di thread runtime.
struct TraceEvent {
    enum Kind { Spawn, Exit, Drop, Reject } kind;
    Pid pid;
    Term reason;
};
using Tracer = std::function<void(const TraceEvent&)>;
using Router = std::function<void(Pid from, Pid to, Term body)>;

// Pid memuat nomor shard di 8 bit teratas (0 = runtime tunggal).
constexpr unsigned kShardShift = 56;
inline unsigned shard_of(Pid p) { return static_cast<unsigned>(p.id >> kShardShift); }

class Process {
public:
    Pid self() const { return pid_; }
    Pid parent() const { return parent_; }
    Runtime& runtime() { return rt_; }
    std::uint64_t reductions() const { return reductions_; }
    std::size_t mailbox_size() const { return mailbox_.size(); }
    bool trap_exit() const { return trap_exit_; }
    void set_trap_exit(bool v) { trap_exit_ = v; }
    void reduce(std::uint32_t n) { extra_ += n; }  // bebankan kerja berat ke budget

    inline void send(Pid to, Term body);
    inline std::optional<Message> receive();
    inline std::optional<Message> receive(const MessagePred& pred);
    inline Pid spawn(Body body, SpawnOptions opts = {});
    inline Pid spawn_link(Body body, SpawnOptions opts = {});
    inline void link(Pid other);
    inline void unlink(Pid other);
    inline Ref monitor(Pid target);
    inline void demonitor(Ref ref);
    inline void exit(Pid target, Term reason);
    inline bool register_name(const std::string& name);
    inline TimerId send_after(std::uint64_t ms, Term body);
    Status stop(Term reason) { exit_reason_ = std::move(reason); return Status::Exit; }

private:
    friend class Runtime;
    Process(Runtime& rt, Pid pid, Body body) : rt_(rt), pid_(pid), body_(std::move(body)) {}

    void reset(Pid pid, Body body) {
        pid_ = pid;
        body_ = std::move(body);
        parent_ = Pid{};
        trap_exit_ = false;
        name_.clear();
        links_.clear();
        monitored_by_.clear();
        monitoring_.clear();
        mailbox_.clear();
        state_ = ProcState::Runnable;
        fresh_ = false;
        exit_reason_ = atom("normal");
        reductions_ = 0;
        extra_ = 0;
    }

    Runtime& rt_;
    Pid pid_;
    Pid parent_{};
    Body body_;
    std::deque<Message> mailbox_;
    std::set<Pid> links_;
    std::map<Ref, Pid> monitored_by_;  // ref -> pengamat
    std::map<Ref, Pid> monitoring_;    // ref -> yang diamati
    bool trap_exit_ = false;
    bool fresh_ = false;               // ada pesan masuk selama slice ini
    ProcState state_ = ProcState::Runnable;
    Term exit_reason_ = atom("normal");
    std::string name_;
    std::uint64_t reductions_ = 0;
    std::uint32_t extra_ = 0;
};

class Runtime {
public:
    static constexpr std::uint32_t kReductionBudget = 2000;

    explicit Runtime(unsigned shard = 0) : shard_(shard), next_pid_((std::uint64_t{shard} << kShardShift) | 1) {}
    unsigned shard() const { return shard_; }
    // Batas produksi: 0 = tanpa batas.
    void set_max_processes(std::size_t n) { max_processes_ = n; }
    void set_max_mailbox(std::size_t n) { max_mailbox_ = n; }
    void set_tracer(Tracer t) { tracer_ = std::move(t); }
    void set_router(Router r) { router_ = std::move(r); }
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    Pid spawn(Body body, SpawnOptions opts = {}) {
        if (!opts.name.empty() && names_.count(opts.name)) return Pid{};
        if (max_processes_ && alive_ >= max_processes_) {
            ++stats_.rejected; trace({TraceEvent::Reject, Pid{}, atom("system_limit")});
            return Pid{};
        }
        Pid pid{next_pid_++};
        std::unique_ptr<Process> p;
        if (!pool_.empty()) {
            p = std::move(pool_.back());
            pool_.pop_back();
            p->reset(pid, std::move(body));
        } else {
            p = std::unique_ptr<Process>(new Process(*this, pid, std::move(body)));
        }

        p->parent_ = opts.parent;
        p->trap_exit_ = opts.trap_exit;
        p->name_ = opts.name;
        if (!opts.name.empty()) names_[opts.name] = pid;
        if (opts.link && opts.parent.valid()) {
            if (Process* par = find(opts.parent)) { par->links_.insert(pid); p->links_.insert(opts.parent); }
        }
        procs_.emplace(pid.id, std::move(p));
        ++alive_; ++stats_.spawned;
        runq_.push_back(pid);
        trace({TraceEvent::Spawn, pid, Term()});
        return pid;
    }

    Process* find(Pid pid) const {
        auto it = procs_.find(pid.id);
        return (it == procs_.end() || it->second->state_ == ProcState::Exited) ? nullptr : it->second.get();
    }
    bool alive(Pid pid) const { return find(pid) != nullptr; }
    std::size_t process_count() const { return alive_; }
    Ref make_ref() { return Ref{next_ref_++}; }

    void send(Pid from, Pid to, Term body) {
        if (shard_of(to) != shard_) {
            if (router_) { ++stats_.remote; router_(from, to, std::move(body)); }
            return;
        }
        Process* t = find(to);
        if (!t) return;
        ++stats_.messages;
        enqueue(*t, Message{from, std::move(body)});
    }
    bool send_named(Pid from, const std::string& name, Term body) {
        auto p = whereis(name);
        if (!p) return false;
        send(from, *p, std::move(body));
        return true;
    }
    std::optional<Pid> whereis(const std::string& name) const {
        auto it = names_.find(name);
        if (it == names_.end()) return std::nullopt;
        return it->second;
    }
    std::vector<std::string> registered_names() const {
        std::vector<std::string> out;
        for (const auto& [k, v] : names_) out.push_back(k);
        return out;
    }
    bool register_name(Process& p, const std::string& name) {
        if (name.empty() || names_.count(name) || !p.name_.empty()) return false;
        names_[name] = p.pid_; p.name_ = name;
        return true;
    }

    // Ambil pesan dari mailbox proses dari luar (untuk probe/tes/event loop).
    std::optional<Message> take(Pid pid, const MessagePred& pred) {
        Process* p = find(pid);
        if (!p) return std::nullopt;
        for (auto it = p->mailbox_.begin(); it != p->mailbox_.end(); ++it)
            if (pred(*it)) { Message m = std::move(*it); p->mailbox_.erase(it); return m; }
        return std::nullopt;
    }

    // ---- link / monitor / exit signal ----
    void link(Pid a, Pid b) {
        Process* pa = find(a); Process* pb = find(b);
        if (!pa) return;
        if (!pb) { signal_exit(b, a, atom(shard_of(b) != shard_ ? "noconnection" : "noproc")); return; }
        pa->links_.insert(b); pb->links_.insert(a);
    }
    void unlink(Pid a, Pid b) {
        if (Process* pa = find(a)) pa->links_.erase(b);
        if (Process* pb = find(b)) pb->links_.erase(a);
    }
    Ref monitor(Pid watcher, Pid target) {
        Ref ref = make_ref();
        Process* w = find(watcher); Process* t = find(target);
        if (!w) return ref;
        if (!t) {
            send(target, watcher, tuple({atom("DOWN"), Term(ref), Term(target), atom(shard_of(target) != shard_ ? "noconnection" : "noproc")}));
            return ref;
        }
        t->monitored_by_[ref] = watcher;
        w->monitoring_[ref] = target;
        return ref;
    }
    void demonitor(Pid watcher, Ref ref) {
        Process* w = find(watcher);
        if (!w) return;
        auto it = w->monitoring_.find(ref);
        if (it == w->monitoring_.end()) return;
        if (Process* t = find(it->second)) t->monitored_by_.erase(ref);
        w->monitoring_.erase(it);
    }
    // Semantik sinyal exit ala Erlang: kill = pasti mati ("killed"), trap_exit = jadi pesan,
    // normal diabaikan, selain itu proses ikut mati.
    void signal_exit(Pid from, Pid target, const Term& reason) {
        Process* t = find(target);
        if (!t) return;
        if (is_atom(reason, "kill")) { exit_process(*t, atom("killed")); return; }
        if (t->trap_exit_) {
            enqueue(*t, Message{from, tuple({atom("EXIT"), Term(from), reason})});
            return;
        }
        if (is_atom(reason, "normal")) {
            if (from == target) exit_process(*t, reason);
            return;
        }
        exit_process(*t, reason);
    }
    void exit_signal(Pid from, Pid target, Term reason) { signal_exit(from, target, reason); flush_graveyard(); }

    // ---- timer ----
    TimerId after(std::uint64_t ms, std::function<void()> fn) { return wheel_.schedule(ms, std::move(fn)); }
    TimerId send_after(std::uint64_t ms, Pid to, Term body) {
        return wheel_.schedule(ms, [this, to, b = std::move(body)]() { send(Pid{}, to, b); });
    }
    bool cancel_timer(TimerId id) { return wheel_.cancel(id); }
    std::uint64_t now_ms() const { return wheel_.now(); }
    std::optional<std::uint64_t> next_timer() const { return wheel_.next_expiry(); }
    void advance_time(std::uint64_t to_ms) { wheel_.advance(to_ms); flush_graveyard(); }

    // ---- scheduler ----
    bool has_runnable() {
        while (!runq_.empty()) {
            Process* p = find(runq_.front());
            if (p && p->state_ == ProcState::Runnable) return true;
            runq_.pop_front();
        }
        return false;
    }
    bool run_slice() {
        flush_graveyard();
        while (!runq_.empty()) {
            Pid pid = runq_.front(); runq_.pop_front();
            Process* p = find(pid);
            if (!p || p->state_ != ProcState::Runnable) continue;
            execute(*p);
            flush_graveyard();
            return true;
        }
        return false;
    }
    std::size_t run_until_idle(std::size_t max_slices = static_cast<std::size_t>(-1)) {
        std::size_t n = 0;
        while (n < max_slices && run_slice()) ++n;
        return n;
    }
    const RuntimeStats& stats() const { return stats_; }

private:
    friend class Process;

    void trace(const TraceEvent& e) { if (tracer_) tracer_(e); }

    void enqueue(Process& t, Message m) {
        if (max_mailbox_ && t.mailbox_.size() >= max_mailbox_) {
            ++stats_.dropped; trace({TraceEvent::Drop, t.pid_, Term()});
            return;
        }
        t.mailbox_.push_back(std::move(m));
        t.fresh_ = true;
        if (t.state_ == ProcState::Waiting) { t.state_ = ProcState::Runnable; runq_.push_back(t.pid_); }
    }

    void execute(Process& p) {
        p.state_ = ProcState::Running;
        p.fresh_ = false;
        ++stats_.slices;
        std::uint32_t budget = kReductionBudget;
        Status st = Status::Continue;
        try {
            while (budget > 0) {
                st = p.body_(p);
                ++p.reductions_; ++stats_.reductions;
                std::uint32_t cost = 1 + p.extra_;
                p.extra_ = 0;
                budget = cost >= budget ? 0 : budget - cost;
                if (p.state_ == ProcState::Exited) return;
                if (st != Status::Continue) break;
            }
        } catch (const std::exception& e) {
            exit_process(p, tuple({atom("error"), Term(std::string(e.what()))}));
            return;
        } catch (...) {
            exit_process(p, tuple({atom("error"), atom("unknown")}));
            return;
        }
        if (p.state_ == ProcState::Exited) return;
        switch (st) {
            case Status::Continue:
                ++stats_.preemptions;
                p.state_ = ProcState::Runnable; runq_.push_back(p.pid_); break;
            case Status::Wait:
                if (p.fresh_) { p.state_ = ProcState::Runnable; runq_.push_back(p.pid_); }
                else p.state_ = ProcState::Waiting;
                break;
            case Status::Done: exit_process(p, atom("normal")); break;
            case Status::Exit: exit_process(p, p.exit_reason_); break;
        }
    }

    void exit_process(Process& p, Term reason) {
        if (p.state_ == ProcState::Exited) return;
        p.state_ = ProcState::Exited;
        --alive_; ++stats_.exited;
        if (!is_atom(reason, "normal") && !is_atom(reason, "shutdown")) ++stats_.crashes;
        trace({TraceEvent::Exit, p.pid_, reason});
        if (!p.name_.empty()) names_.erase(p.name_);
        for (auto& [ref, target] : p.monitoring_)
            if (Process* t = find(target)) t->monitored_by_.erase(ref);
        p.monitoring_.clear();
        auto links = std::move(p.links_); p.links_.clear();
        auto mons = std::move(p.monitored_by_); p.monitored_by_.clear();
        for (Pid q : links) {
            if (Process* qp = find(q)) qp->links_.erase(p.pid_);
            signal_exit(p.pid_, q, reason);
        }
        for (auto& [ref, watcher] : mons)
            send(p.pid_, watcher, tuple({atom("DOWN"), Term(ref), Term(p.pid_), reason}));
        p.mailbox_.clear();
        graveyard_.push_back(p.pid_);
    }

    void flush_graveyard() {
        for (Pid pid : graveyard_) {
            auto it = procs_.find(pid.id);
            if (it != procs_.end()) {
                pool_.push_back(std::move(it->second));
                procs_.erase(it);
            }
        }
        graveyard_.clear();
    }

    std::unordered_map<std::uint64_t, std::unique_ptr<Process>> procs_;
    std::vector<std::unique_ptr<Process>> pool_;
    std::unordered_map<std::string, Pid> names_;
    std::deque<Pid> runq_;
    std::vector<Pid> graveyard_;
    TimerWheel wheel_;
    RuntimeStats stats_;
    std::size_t alive_ = 0;
    unsigned shard_ = 0;
    std::size_t max_processes_ = 0, max_mailbox_ = 0;
    Tracer tracer_;
    Router router_;
    std::uint64_t next_pid_ = 1;
    std::uint64_t next_ref_ = 1;
};

// ---- Process: implementasi yang butuh Runtime lengkap ----
inline void Process::send(Pid to, Term body) { rt_.send(pid_, to, std::move(body)); }
inline std::optional<Message> Process::receive() {
    if (mailbox_.empty()) { fresh_ = false; return std::nullopt; }
    Message m = std::move(mailbox_.front());
    mailbox_.pop_front();
    return m;
}
inline std::optional<Message> Process::receive(const MessagePred& pred) {
    for (auto it = mailbox_.begin(); it != mailbox_.end(); ++it) {
        if (pred(*it)) { Message m = std::move(*it); mailbox_.erase(it); return m; }
    }
    return std::nullopt;
}
inline Pid Process::spawn(Body body, SpawnOptions opts) { opts.parent = pid_; return rt_.spawn(std::move(body), std::move(opts)); }
inline Pid Process::spawn_link(Body body, SpawnOptions opts) { opts.link = true; return spawn(std::move(body), std::move(opts)); }
inline void Process::link(Pid other) { rt_.link(pid_, other); }
inline void Process::unlink(Pid other) { rt_.unlink(pid_, other); }
inline Ref Process::monitor(Pid target) { return rt_.monitor(pid_, target); }
inline void Process::demonitor(Ref ref) { rt_.demonitor(pid_, ref); }
inline void Process::exit(Pid target, Term reason) { rt_.signal_exit(pid_, target, reason); }
inline bool Process::register_name(const std::string& name) { return rt_.register_name(*this, name); }
inline TimerId Process::send_after(std::uint64_t ms, Term body) { return rt_.send_after(ms, pid_, std::move(body)); }

}  // namespace garuda::beam
