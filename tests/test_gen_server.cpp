#include "../include/garuda/beam/otp/gen_server.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <optional>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

class CounterGenServer : public GenServer {
private:
    int counter_ = 0;

public:
    void init(Process& /*p*/) override {
        counter_ = 100;
    }

    CallResult handle_call(Process& /*p*/, const Term& request, Pid /*from*/) override {
        if (is_atom(request, "get")) {
            return CallResult{Term(static_cast<std::int64_t>(counter_)), std::nullopt};
        }
        if (is_atom(request, "stop")) {
            return CallResult{atom("ok"), atom("normal")};
        }
        return CallResult{atom("unknown_call"), std::nullopt};
    }

    void handle_cast(Process& /*p*/, const Term& msg) override {
        if (is_atom(msg, "inc")) {
            counter_ += 1;
        } else if (is_atom(msg, "dec")) {
            counter_ -= 1;
        }
    }

    void handle_info(Process& /*p*/, const Term& msg) override {
        if (is_atom(msg, "reset_info")) {
            counter_ = 0;
        }
    }

    void terminate(Process& /*p*/, const Term& reason) override {
        std::cout << "[CounterGenServer] Terminated with reason: " << to_string(reason) << "\n";
    }
};

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G1 - GARUDA GEN_SERVER TESTS\n";
    std::cout << "========================================\n\n";

    Runtime rt;

    // Test 1: Start GenServer, call & cast
    std::cout << "[Test 1] Start GenServer, call & cast...\n";
    {
        auto srv = std::make_shared<CounterGenServer>();
        Pid pid = start_gen_server(rt, srv);
        CHECK(pid.valid(), "Pid GenServer harus valid");

        rt.run_until_idle();

        // Send gen_call ("get")
        auto ref1 = std::make_shared<std::optional<Ref>>();
        rt.spawn([pid, ref1](Process& p) -> Status {
            if (!ref1->has_value()) {
                *ref1 = gen_call(p, pid, atom("get"));
            }
            auto reply = gen_take_reply(p, **ref1);
            if (reply) {
                const std::int64_t* val = reply->get_if<std::int64_t>();
                CHECK(val && *val == 100, "Initial counter value harus 100");
                return Status::Done;
            }
            return Status::Wait;
        });

        rt.run_until_idle();

        // Async cast "inc"
        rt.spawn([pid](Process& p) -> Status {
            gen_cast(p, pid, atom("inc"));
            return Status::Done;
        });

        rt.run_until_idle();

        // Verify value incremented to 101
        auto ref2 = std::make_shared<std::optional<Ref>>();
        rt.spawn([pid, ref2](Process& p) -> Status {
            if (!ref2->has_value()) {
                *ref2 = gen_call(p, pid, atom("get"));
            }
            auto reply = gen_take_reply(p, **ref2);
            if (reply) {
                const std::int64_t* val = reply->get_if<std::int64_t>();
                CHECK(val && *val == 101, "Counter setelah inc harus 101");
                return Status::Done;
            }
            return Status::Wait;
        });

        rt.run_until_idle();
    }
    std::cout << "PASS\n\n";

    // Test 2: handle_info messaging
    std::cout << "[Test 2] handle_info message...\n";
    {
        auto srv = std::make_shared<CounterGenServer>();
        Pid pid = start_gen_server(rt, srv);
        rt.run_until_idle();

        // Send raw info message
        rt.send(Pid{}, pid, atom("reset_info"));
        rt.run_until_idle();

        // Check counter is reset to 0
        auto ref3 = std::make_shared<std::optional<Ref>>();
        rt.spawn([pid, ref3](Process& p) -> Status {
            if (!ref3->has_value()) {
                *ref3 = gen_call(p, pid, atom("get"));
            }
            auto reply = gen_take_reply(p, **ref3);
            if (reply) {
                const std::int64_t* val = reply->get_if<std::int64_t>();
                CHECK(val && *val == 0, "Counter setelah reset_info harus 0");
                return Status::Done;
            }
            return Status::Wait;
        });

        rt.run_until_idle();
    }
    std::cout << "PASS\n\n";

    // Test 3: Stop server via handle_call
    std::cout << "[Test 3] Stop GenServer via call...\n";
    {
        auto srv = std::make_shared<CounterGenServer>();
        Pid pid = start_gen_server(rt, srv);
        rt.run_until_idle();

        auto ref4 = std::make_shared<std::optional<Ref>>();
        rt.spawn([pid, ref4](Process& p) -> Status {
            if (!ref4->has_value()) {
                *ref4 = gen_call(p, pid, atom("stop"));
            }
            auto reply = gen_take_reply(p, **ref4);
            if (reply) {
                CHECK(is_atom(*reply, "ok"), "Reply stop harus ok");
                return Status::Done;
            }
            return Status::Wait;
        });

        rt.run_until_idle();
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G1 - GEN_SERVER SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
