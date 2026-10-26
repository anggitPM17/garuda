#pragma once
// Node: N shard, tiap shard = 1 thread + Runtime + EventLoop sendiri (share-nothing).
// Pid membawa nomor shard; send() lintas shard aman dari thread mana pun.
// Batasan yang disengaja: link/monitor/exit lintas shard TIDAK didukung (monitor -> DOWN
// noconnection); gunakan pesan biasa untuk koordinasi antar shard.
#include "event_loop.hpp"

#include <future>
#include <thread>

namespace garuda::beam {

class Node {
public:
    explicit Node(unsigned shards, ClockMode mode = ClockMode::Real) {
        if (shards == 0) shards = 1;
        if (shards > 255) shards = 255;
        for (unsigned i = 0; i < shards; ++i) shards_.emplace_back(new Shard(i, mode));
        for (auto& s : shards_) {
            s->loop.set_keep_alive(true);
            s->rt.set_router([this](Pid from, Pid to, Term body) { route(from, to, std::move(body)); });
        }
    }
    ~Node() { stop(); }
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    unsigned shards() const { return static_cast<unsigned>(shards_.size()); }
    // Hanya aman dipakai sebelum start() (konfigurasi limit/tracer).
    Runtime& runtime_unsafe(unsigned shard) { return shards_.at(shard)->rt; }

    void start() {
        if (started_) return;
        started_ = true;
        for (auto& s : shards_) { Shard* sp = s.get(); s->th = std::thread([sp] { sp->loop.run(); }); }
    }
    void stop() {
        if (!started_) return;
        started_ = false;
        for (auto& s : shards_) s->loop.stop();
        for (auto& s : shards_) if (s->th.joinable()) s->th.join();
    }

    // Blok sampai proses lahir. JANGAN dipanggil dari dalam thread shard (deadlock);
    // dari dalam proses gunakan Process::spawn.
    // Sebelum start(): dijalankan langsung (belum ada thread).
    Pid spawn_on(unsigned shard, Body body, SpawnOptions opts = {}) {
        if (!started_) return shards_.at(shard)->rt.spawn(std::move(body), std::move(opts));
        auto pr = std::make_shared<std::promise<Pid>>();
        auto fut = pr->get_future();
        shards_.at(shard)->loop.post([pr, b = std::move(body), o = std::move(opts)](Runtime& r) mutable {
            pr->set_value(r.spawn(std::move(b), std::move(o)));
        });
        return fut.get();
    }
    // Aman dari thread mana pun.
    void send(Pid to, Term body) { route(Pid{}, to, std::move(body)); }

    Pid whereis_on(unsigned shard, const std::string& name) {
        if (shard >= shards_.size()) return Pid{};
        if (!started_) {
            auto opt = shards_.at(shard)->rt.whereis(name);
            return opt ? *opt : Pid{};
        }
        auto pr = std::make_shared<std::promise<Pid>>();
        auto fut = pr->get_future();
        shards_.at(shard)->loop.post([pr, name](Runtime& r) {
            auto opt = r.whereis(name);
            pr->set_value(opt ? *opt : Pid{});
        });
        return fut.get();
    }

    // Snapshot statistik (diambil di thread masing-masing shard).
    RuntimeStats stats() {
        RuntimeStats total;
        for (auto& s : shards_) {
            if (!started_) { add(total, s->rt.stats()); continue; }
            auto pr = std::make_shared<std::promise<RuntimeStats>>();
            auto fut = pr->get_future();
            s->loop.post([pr](Runtime& r) { pr->set_value(r.stats()); });
            add(total, fut.get());
        }
        return total;
    }

private:
    struct Shard {
        Shard(unsigned i, ClockMode m) : rt(i), loop(rt, m) {}
        Runtime rt;
        EventLoop loop;
        std::thread th;
    };
    static void add(RuntimeStats& t, const RuntimeStats& x) {
        t.spawned += x.spawned; t.exited += x.exited; t.messages += x.messages;
        t.reductions += x.reductions; t.slices += x.slices; t.preemptions += x.preemptions;
        t.dropped += x.dropped; t.rejected += x.rejected; t.remote += x.remote; t.crashes += x.crashes;
    }
    void route(Pid from, Pid to, Term body) {
        unsigned s = shard_of(to);
        if (s >= shards_.size()) return;
        shards_[s]->loop.post([from, to, b = std::move(body)](Runtime& r) { r.send(from, to, b); });
    }
    std::vector<std::unique_ptr<Shard>> shards_;
    bool started_ = false;
};

}  // namespace garuda::beam
