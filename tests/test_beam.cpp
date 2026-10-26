#include "garuda/beam/beam.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

#if !defined(_WIN32)
#include <unistd.h>
#endif

using namespace garuda::beam;

static int g_fail = 0, g_checks = 0;
#define CHECK(c) do { ++g_checks; if (!(c)) { ++g_fail; std::printf("  GAGAL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)
#define TEST(name) static void name(); struct R_##name { R_##name() { tests().push_back({#name, name}); } } r_##name; static void name()
struct T { const char* n; void (*f)(); };
static std::vector<T>& tests() { static std::vector<T> v; return v; }

inline void sleep_us(std::uint64_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

TEST(term_order_and_print) {
    Term i(42), a(atom("ok")), s("abc"), p(Pid{5}), r(Ref{2});
    Term t = tuple({atom("ok"), Term(10)});
    Term l = list({Term(1), Term(2)});
    CHECK(i < a && a < r && r < p && p < t && t < l && l < s);
    CHECK(to_string(i) == "42");
    CHECK(to_string(a) == "ok");
    CHECK(to_string(s) == "\"abc\"");
    CHECK(to_string(p) == "<0.5>");
    CHECK(to_string(t) == "{ok,10}");
    CHECK(to_string(l) == "[1,2]");
}

TEST(timer_wheel_cascade) {
    TimerWheel tw;
    bool b1 = false, b2 = false, b3 = false;
    tw.after(10, [&] { b1 = true; });
    tw.after(300, [&] { b2 = true; });
    tw.after(100000, [&] { b3 = true; });
    CHECK(tw.advance_to(5) == 0); CHECK(!b1);
    CHECK(tw.advance_to(10) == 1); CHECK(b1);
    CHECK(tw.advance_to(299) == 0); CHECK(!b2);
    CHECK(tw.advance_to(300) == 1); CHECK(b2);
    CHECK(tw.advance_to(99999) == 0); CHECK(!b3);
    CHECK(tw.advance_to(100000) == 1); CHECK(b3);
}

TEST(selective_receive_and_reductions) {
    Runtime rt;
    std::string order;
    Pid p = rt.spawn([&](Process& self) {
        auto m1 = self.receive([](const Message& m) { return is_atom(m.body, "b"); });
        if (!m1) return Status::Wait;
        order += "b";
        auto m2 = self.receive();
        if (!m2) return Status::Wait;
        order += is_atom(m2->body, "a") ? "a" : "?";
        return Status::Done;
    });
    rt.send(Pid{}, p, atom("a"));
    rt.send(Pid{}, p, atom("b"));
    CHECK(rt.run_slice());
    CHECK(order == "ba");
    CHECK(rt.stats().reductions > 0);
    CHECK(rt.stats().slices > 0);
}

TEST(preemption_fairness) {
    Runtime rt;
    std::string trace;
    auto make_heavy = [&](char id) {
        return [id, &trace, count = 0](Process& self) mutable {
            trace += id;
            self.reduce(800);
            if (++count < 5) return Status::Continue;
            return Status::Done;
        };
    };
    rt.spawn(make_heavy('A'));
    rt.spawn(make_heavy('B'));
    rt.run_until_idle();
    CHECK(trace.find("A") != std::string::npos);
    CHECK(trace.find("B") != std::string::npos);
    CHECK(rt.stats().slices >= 2);
}

TEST(link_trap_exit_and_cascade) {
    Runtime rt;
    std::vector<std::string> log;
    Pid p1 = rt.spawn([&](Process& self) {
        self.set_trap_exit(true);
        auto m = self.receive();
        if (!m) return Status::Wait;
        log.push_back(to_string(m->body));
        return Status::Done;
    });
    rt.spawn([&](Process& self) {
        self.spawn_link([](Process& p) { return p.stop(atom("bad")); }, { .parent = self.self() });
        return Status::Wait;
    }, { .link = true, .parent = p1 });

    rt.run_until_idle();
    CHECK(log.size() == 1);
    CHECK(log[0].find("EXIT") != std::string::npos || log[0].find("bad") != std::string::npos);
}

TEST(monitor_down_signal) {
    Runtime rt;
    Term down;
    rt.spawn([&](Process& self) {
        Pid target = self.spawn([](Process& p) { return p.stop(atom("crash")); });
        self.monitor(target);
        auto m = self.receive();
        if (!m) return Status::Wait;
        down = m->body;
        return Status::Done;
    });
    rt.run_until_idle();
    CHECK(tuple_size(down) == 5);
    CHECK(is_atom(*tuple_at(down, 0), "DOWN"));
}

TEST(registry_name) {
    Runtime rt;
    Pid p = rt.spawn([&](Process& self) {
        CHECK(self.register_name("bob"));
        CHECK(!self.register_name("bob"));
        return Status::Wait;
    });
    rt.run_slice();
    CHECK(rt.whereis("bob") == p);
    rt.exit_signal(Pid{}, p, atom("kill"));
    rt.run_until_idle();
    CHECK(!rt.whereis("bob").has_value());
}

TEST(gen_server_call_cast) {
    Runtime rt;
    EventLoop loop(rt, ClockMode::Virtual);
    struct St : GenServer {
        std::int64_t n = 0;
        CallResult handle_call(Process&, const Term& req, Pid) override {
            if (is_atom(req, "get")) return {Term(n), std::nullopt};
            return {atom("bad"), std::nullopt};
        }
        void handle_cast(Process&, const Term& m) override {
            if (auto* i = m.get_if<std::int64_t>()) n += *i;
        }
    };
    Pid gs = start_gen_server(rt, std::make_shared<St>());
    gen_cast(rt, gs, Term(10));
    gen_cast(rt, gs, Term(5));
    auto r = loop.call(gs, atom("get"));
    CHECK(r && r->get_if<std::int64_t>() && *r->get_if<std::int64_t>() == 15);
}

TEST(supervisor_one_for_one) {
    Runtime rt;
    std::atomic<int> starts{0};
    ChildSpec spec{"w1", [&](Process& sup) {
        int id = ++starts;
        return sup.spawn_link([id](Process& p) { return id == 1 ? p.stop(atom("crash")) : Status::Wait; });
    }};
    Pid sup = start_supervisor(rt, SupervisorFlags{.max_restarts = 5}, {spec});
    rt.run_until_idle();
    CHECK(starts == 2);
    CHECK(rt.alive(sup));
}

#if !defined(_WIN32)
TEST(event_loop_watch_read) {
    Runtime rt; EventLoop loop(rt, ClockMode::Virtual);
    int fds[2]; CHECK(::pipe(fds) == 0);
    std::string got;
    Pid p = rt.spawn([&](Process& self) {
        auto m = self.receive();
        if (!m) return Status::Wait;
        char buf[16]; ssize_t n = ::read(fds[0], buf, sizeof buf);
        got.assign(buf, n > 0 ? n : 0);
        return Status::Done;
    });
    loop.watch_read(fds[0], p, atom("pipe"));
    CHECK(::write(fds[1], "halo", 4) == 4);
    loop.run_until([&] { return !got.empty(); }, 100);
    CHECK(got == "halo");
    loop.unwatch(fds[0]);
    ::close(fds[0]); ::close(fds[1]);
}
#endif

TEST(post_from_other_thread) {
    Runtime rt; EventLoop loop(rt, ClockMode::Real);
    loop.set_keep_alive(true);
    std::atomic<int> got{0};
    Pid p = rt.spawn([&](Process& self) {
        auto m = self.receive();
        if (!m) return Status::Wait;
        got = static_cast<int>(*m->body.get_if<std::int64_t>());
        loop.stop();
        return Status::Done;
    });
    std::thread t([&] { sleep_us(20000); loop.post([p](Runtime& r) { r.send(Pid{}, p, Term(77)); }); });
    loop.run();
    t.join();
    CHECK(got == 77);
}

TEST(ets_lite) {
    Ets ets;
    auto& s = ets.ordered_set("tab");
    s.insert(Term(10), Term("a"));
    s.insert(Term(5), Term("b"));
    CHECK(s.lookup(Term(5)) && *s.lookup(Term(5)) == Term("b"));
    CHECK(s.first() == Term(5));
    CHECK(s.next(Term(5)) == Term(10));
}

TEST(limits_process_and_mailbox) {
    Runtime rt;
    rt.set_max_processes(2);
    rt.set_max_mailbox(1);
    Pid p1 = rt.spawn([](Process&) { return Status::Wait; });
    Pid p2 = rt.spawn([](Process&) { return Status::Wait; });
    Pid p3 = rt.spawn([](Process&) { return Status::Wait; });
    CHECK(p1.valid() && p2.valid() && !p3.valid());
    CHECK(rt.stats().rejected == 1);

    rt.send(Pid{}, p1, Term(1));
    rt.send(Pid{}, p1, Term(2));
    CHECK(rt.stats().dropped == 1);
}

TEST(task_async_await) {
    Runtime rt;
    EventLoop loop(rt, ClockMode::Virtual);
    Ref ref;
    rt.spawn([&ref](Process& p) {
        ref = task_async(p, []() { return Term(123); });
        return Status::Wait;
    });
    rt.run_until_idle();
    std::optional<Term> val;
    rt.spawn([&val, ref](Process& p) {
        val = task_await(p, ref);
        return Status::Done;
    });
    rt.run_until_idle();
    CHECK(val && val->get_if<std::int64_t>() && *val->get_if<std::int64_t>() == 123);
}

TEST(node_inter_shard_send) {
    Node node(2);
    std::atomic<int> got{0};
    Pid client = node.spawn_on(0, [&](Process& p) {
        p.send(node.whereis_on(1, "srv"), Term(42));
        return Status::Done;
    });
    node.spawn_on(1, [&](Process& p) {
        p.register_name("srv");
        auto m = p.receive();
        if (!m) return Status::Wait;
        got = static_cast<int>(*m->body.get_if<std::int64_t>());
        return Status::Done;
    });
    CHECK(shard_of(client) == 0);
    node.start();
    for (int i = 0; i < 200 && got == 0; ++i) sleep_us(5000);
    CHECK(got == 42);
    CHECK(node.stats().remote >= 2);
    node.stop();
}

TEST(node_monitor_cross_shard_is_noconnection) {
    Node node(2);
    Pid far_pid = node.spawn_on(1, [](Process&) { return Status::Wait; });
    std::atomic<bool> ok{false};
    node.spawn_on(0, [far_pid, &ok](Process& p) {
        p.monitor(far_pid);
        auto m = p.receive();
        if (!m) return Status::Wait;
        ok = is_atom(*tuple_at(m->body, 3), "noconnection");
        return Status::Done;
    });
    node.start();
    for (int i = 0; i < 200 && !ok; ++i) sleep_us(5000);
    CHECK(ok);
    node.stop();
}

TEST(chaos_invariants) {
    Runtime rt;
    rt.set_max_processes(4000); rt.set_max_mailbox(64);
    std::uint64_t spawn_ev = 0, exit_ev = 0;
    rt.set_tracer([&](const TraceEvent& e) {
        if (e.kind == TraceEvent::Spawn) ++spawn_ev;
        if (e.kind == TraceEvent::Exit) ++exit_ev;
    });

    std::uint64_t seed = 12345;
    auto rnd = [&] { seed = seed * 6364136223846793005ULL + 1; return seed; };

    std::vector<Pid> pool;
    for (int step = 0; step < 1000; ++step) {
        uint32_t op = rnd() % 100;
        if (op < 40 && pool.size() < 2000) {
            Pid parent = pool.empty() ? Pid{} : pool[rnd() % pool.size()];
            Pid child = rt.spawn([&, lives = rnd() % 20](Process& self) mutable {
                if (rnd() % 10 == 0 && !self.runtime().registered_names().empty()) {
                    auto names = self.runtime().registered_names();
                    self.runtime().whereis(names[rnd() % names.size()]);
                }
                if (lives == 0) return Status::Done;
                --lives;
                return Status::Continue;
            }, { .link = (rnd() % 2 == 0), .parent = parent });
            if (child.valid()) pool.push_back(child);
        } else if (op < 70 && !pool.empty()) {
            Pid src = pool[rnd() % pool.size()];
            Pid dst = pool[rnd() % pool.size()];
            rt.send(src, dst, Term(static_cast<int64_t>(rnd() % 1000)));
        } else if (op < 90 && !pool.empty()) {
            Pid victim = pool[rnd() % pool.size()];
            rt.exit_signal(Pid{}, victim, atom("chaos_kill"));
        }
        rt.run_slice();
    }
    rt.run_until_idle();

    const auto& s = rt.stats();
    CHECK(s.spawned == s.exited);
    CHECK(spawn_ev == s.spawned);
    CHECK(exit_ev == s.exited);
}

int main() {
    std::printf("Menjalankan %zu tes Garuda...\n", tests().size());
    for (auto& t : tests()) {
        int f_before = g_fail;
        t.f();
        if (g_fail == f_before) std::printf("  [PASS] %s\n", t.n);
        else std::printf("  [FAIL] %s\n", t.n);
    }
    std::printf("=======================================\n");
    std::printf("Hasil: %d gagal dari %d pengecekan.\n", g_fail, g_checks);
    return g_fail == 0 ? 0 : 1;
}
