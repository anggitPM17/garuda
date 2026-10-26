#include "garuda/beam/beam.hpp"
#include <chrono>
#include <cstdio>

using namespace garuda::beam;
using Clock = std::chrono::steady_clock;
static double secs(Clock::time_point a) { return std::chrono::duration<double>(Clock::now() - a).count(); }

int main() {
    // 1) spawn + exit 1.000.000 proses (batch 10k)
    {
        Runtime rt;
        auto t = Clock::now();
        for (int b = 0; b < 100; ++b) {
            for (int i = 0; i < 10000; ++i) rt.spawn([](Process&) { return Status::Done; });
            rt.run_until_idle();
        }
        double s = secs(t);
        std::printf("spawn+exit 1.000.000 proses : %.2f dtk  (%.0f proses/dtk)\n", s, 1e6 / s);
    }
    // 2) ring 100.000 proses, 1.000.000 hop pesan
    {
        Runtime rt;
        const int N = 100000; const long HOPS = 1000000;
        std::vector<Pid> ring(N);
        long done_hops = 0;
        for (int i = 0; i < N; ++i)
            ring[i] = rt.spawn([&, i](Process& p) {
                auto m = p.receive();
                if (!m) return Status::Wait;
                long left = *m->body.get_if<std::int64_t>();
                ++done_hops;
                if (left > 0) p.send(ring[(i + 1) % N], Term(left - 1));
                return Status::Continue;
            });
        rt.run_until_idle();
        auto t = Clock::now();
        rt.send(Pid{}, ring[0], Term(HOPS - 1));
        rt.run_until_idle();
        double s = secs(t);
        std::printf("ring %d proses, %ld hop      : %.2f dtk  (%.0f pesan/dtk)\n", N, done_hops, s, done_hops / s);
    }
    // 3) 200.000 timer tersebar, virtual time
    {
        Runtime rt; EventLoop loop(rt, ClockMode::Virtual);
        long fired = 0;
        auto t = Clock::now();
        for (int i = 0; i < 200000; ++i) rt.after(1 + (i * 7919) % 3600000, [&] { ++fired; });
        loop.run();
        double s = secs(t);
        std::printf("timer 200.000 (jangkauan 1 jam): %.2f dtk  (terpicu %ld, waktu virtual %llu ms)\n",
                    s, fired, (unsigned long long)rt.now_ms());
    }
}
