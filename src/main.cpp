#include "../include/garuda.hpp"
#include "../include/garuda/beam/beam.hpp"
#include <iostream>

using namespace garuda::beam;

struct KVStore : GenServer {
    OrderedSet table;
    CallResult handle_call(Process&, const Term& req, Pid) override {
        if (tuple_size(req) == 2 && is_atom(*tuple_at(req, 0), "get")) {
            auto v = table.lookup(*tuple_at(req, 1));
            return {v ? tuple({atom("ok"), *v}) : atom("not_found"), std::nullopt};
        }
        return {atom("bad_request"), std::nullopt};
    }
    void handle_cast(Process&, const Term& m) override {
        if (tuple_size(m) == 3 && is_atom(*tuple_at(m, 0), "put")) table.insert(*tuple_at(m, 1), *tuple_at(m, 2));
    }
};

int main() {
    std::cout << "=======================================================\n";
    std::cout << GARUDA_ENGINE_NAME << " v" << GARUDA_VERSION_STRING << "\n";
    std::cout << "=======================================================\n\n";
    GarudaUltraKernel kernel;

    Runtime rt;
    EventLoop loop(rt, ClockMode::Virtual);

    // Supervisor: worker yang crash dua kali lalu hidup stabil.
    int starts = 0;
    ChildSpec flaky{"flaky", [&](Process& sup) {
        int run = ++starts;
        return sup.spawn_link([run](Process& p) { return run <= 2 ? p.stop(atom("crash")) : Status::Wait; });
    }};
    Pid sup = start_supervisor(rt, SupervisorFlags{}, {flaky});

    // gen_server key-value
    Pid kv = start_gen_server(rt, std::make_shared<KVStore>());
    gen_cast(rt, kv, tuple({atom("put"), atom("negara"), "Indonesia"}));
    auto reply = loop.call(kv, tuple({atom("get"), atom("negara")}));

    // timer 2,5 detik waktu virtual
    Pid clock_p = rt.spawn([](Process& p) {
        auto m = p.receive();
        if (!m) return Status::Wait;
        std::cout << "[timer] pesan " << to_string(m->body) << " pada t=" << p.runtime().now_ms() << " ms\n";
        return Status::Done;
    });
    rt.send_after(2500, clock_p, atom("tick"));
    loop.run_until([&] { return !rt.alive(clock_p); }, 10000);

    std::cout << "[supervisor] anak distart " << starts << "x, supervisor " << (rt.alive(sup) ? "hidup" : "mati") << "\n";
    std::cout << "[gen_server] get(negara) = " << (reply ? to_string(*reply) : "timeout") << "\n";
    const auto& s = rt.stats();
    std::cout << "[stats] spawn=" << s.spawned << " exit=" << s.exited << " pesan=" << s.messages
              << " reduction=" << s.reductions << " slice=" << s.slices << "\n";
    std::cout << "\n[SYSTEM] Garuda v" << GARUDA_VERSION_STRING << " berjalan stabil.\n";
    return 0;
}
