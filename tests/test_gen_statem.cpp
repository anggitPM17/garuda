#include "../include/garuda/beam/otp/gen_statem.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <atomic>
#include <vector>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

class TrafficLight : public GenStatem {
public:
    std::atomic<int> transition_count{0};
    std::atomic<int> terminate_count{0};
    std::string last_terminate_reason;

    void init(Process&, Term& state, Term& data) override {
        state = atom("red");
        data = atom("none");
    }

    StatemNextState handle_event(Process&, const Term& event, const Term& state, const Term& data) override {
        StatemNextState result;
        result.data = data;

        if (is_atom(event, "next")) {
            transition_count.fetch_add(1);
            if (is_atom(state, "red")) result.state = atom("green");
            else if (is_atom(state, "green")) result.state = atom("yellow");
            else if (is_atom(state, "yellow")) result.state = atom("red");
            else result.state = state;
            result.reply = atom("ok");
        } else if (is_atom(event, "stop")) {
            result.state = state;
            result.stop = atom("normal");
            result.reply = atom("stopped");
        } else if (is_atom(event, "get_state")) {
            result.state = state;
            result.reply = state;
        } else {
            result.state = state;
            result.reply = atom("unknown_event");
        }
        return result;
    }

    void terminate(Process&, const Term& reason, const Term&, const Term&) override {
        terminate_count.fetch_add(1);
        last_terminate_reason = is_atom(reason, "normal") ? "normal" : "abnormal";
    }
};

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G1.5 - GARUDA GEN_STATEM TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1
    std::cout << "[Test 1] Init & state transition...\n";
    {
        Runtime rt;
        auto tl = std::make_shared<TrafficLight>();
        Pid server = start_gen_statem(rt, tl, atom("idle"), atom("none"));
        rt.run_until_idle();
        CHECK(server.valid(), "Server pid harus valid");

        rt.spawn([server](Process& p) -> Status {
            gen_statem_cast(p, server, atom("next"));
            return Status::Done;
        });
        rt.run_until_idle();
        CHECK(tl->transition_count.load() == 1, "1 transisi (red -> green)");

        auto ref = std::make_shared<std::optional<Ref>>();
        std::string final_state;
        rt.spawn([server, ref, &final_state](Process& p) -> Status {
            if (!ref->has_value()) *ref = gen_statem_call(p, server, atom("get_state"));
            auto reply = gen_statem_take_reply(p, **ref);
            if (reply) { final_state = is_atom(*reply, "green") ? "green" : "other"; return Status::Done; }
            return Status::Wait;
        });
        rt.run_until_idle();
        CHECK(final_state == "green", "State final harus green");
    }
    std::cout << "PASS\n\n";

    // Test 2
    std::cout << "[Test 2] Multiple transitions...\n";
    {
        Runtime rt;
        auto tl = std::make_shared<TrafficLight>();
        Pid server = start_gen_statem(rt, tl);
        rt.run_until_idle();

        for (int i = 0; i < 3; ++i) {
            rt.spawn([server](Process& p) -> Status {
                gen_statem_cast(p, server, atom("next"));
                return Status::Done;
            });
            rt.run_until_idle();
        }
        CHECK(tl->transition_count.load() == 3, "3 transisi total");

        auto ref = std::make_shared<std::optional<Ref>>();
        std::string final_state;
        rt.spawn([server, ref, &final_state](Process& p) -> Status {
            if (!ref->has_value()) *ref = gen_statem_call(p, server, atom("get_state"));
            auto reply = gen_statem_take_reply(p, **ref);
            if (reply) { final_state = is_atom(*reply, "red") ? "red" : "other"; return Status::Done; }
            return Status::Wait;
        });
        rt.run_until_idle();
        CHECK(final_state == "red", "Setelah 3 transisi, state kembali ke red");
    }
    std::cout << "PASS\n\n";

    // Test 3
    std::cout << "[Test 3] Data preservation across transitions...\n";
    {
        class CounterStatem : public GenStatem {
        public:
            void init(Process&, Term& state, Term& data) override {
                state = atom("counting");
                data = Term(static_cast<std::int64_t>(0));
            }
            StatemNextState handle_event(Process&, const Term& event, const Term& state, const Term& data) override {
                StatemNextState result;
                result.state = state;
                result.data = data;
                if (is_atom(event, "increment")) {
                    const std::int64_t* d = data.get_if<std::int64_t>();
                    std::int64_t current = d ? *d : 0;
                    result.data = Term(current + 1);
                    result.reply = Term(current + 1);
                } else if (is_atom(event, "get")) {
                    result.reply = data;
                }
                return result;
            }
        };

        Runtime rt;
        auto cs = std::make_shared<CounterStatem>();
        Pid server = start_gen_statem(rt, cs);
        rt.run_until_idle();

        for (int i = 0; i < 3; ++i) {
            rt.spawn([server](Process& p) -> Status {
                gen_statem_cast(p, server, atom("increment"));
                return Status::Done;
            });
            rt.run_until_idle();
        }

        auto ref = std::make_shared<std::optional<Ref>>();
        std::int64_t final_count = 0;
        rt.spawn([server, ref, &final_count](Process& p) -> Status {
            if (!ref->has_value()) *ref = gen_statem_call(p, server, atom("get"));
            auto reply = gen_statem_take_reply(p, **ref);
            if (reply) { const std::int64_t* v = reply->get_if<std::int64_t>(); if (v) final_count = *v; return Status::Done; }
            return Status::Wait;
        });
        rt.run_until_idle();
        CHECK(final_count == 3, "Counter harus 3 setelah 3 increment");
    }
    std::cout << "PASS\n\n";

    // Test 4
    std::cout << "[Test 4] Unknown event...\n";
    {
        Runtime rt;
        auto tl = std::make_shared<TrafficLight>();
        Pid server = start_gen_statem(rt, tl);
        rt.run_until_idle();

        rt.spawn([server](Process& p) -> Status {
            gen_statem_cast(p, server, atom("unknown_event"));
            return Status::Done;
        });
        rt.run_until_idle();

        auto ref = std::make_shared<std::optional<Ref>>();
        std::string final_state;
        rt.spawn([server, ref, &final_state](Process& p) -> Status {
            if (!ref->has_value()) *ref = gen_statem_call(p, server, atom("get_state"));
            auto reply = gen_statem_take_reply(p, **ref);
            if (reply) { final_state = is_atom(*reply, "red") ? "red" : "other"; return Status::Done; }
            return Status::Wait;
        });
        rt.run_until_idle();
        CHECK(final_state == "red", "State tetap red setelah unknown event");
        CHECK(tl->transition_count.load() == 0, "Tidak ada transisi (unknown event tidak dihitung)");
    }
    std::cout << "PASS\n\n";

    // Test 5
    std::cout << "[Test 5] Terminate...\n";
    {
        Runtime rt;
        auto tl = std::make_shared<TrafficLight>();
        Pid server = start_gen_statem(rt, tl);
        rt.run_until_idle();

        rt.spawn([server](Process& p) -> Status {
            gen_statem_cast(p, server, atom("stop"));
            return Status::Done;
        });
        rt.run_until_idle();
        CHECK(tl->terminate_count.load() == 1, "terminate dipanggil 1x");
        CHECK(tl->last_terminate_reason == "normal", "Reason harus 'normal'");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G1.5 - GEN_STATEM SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
