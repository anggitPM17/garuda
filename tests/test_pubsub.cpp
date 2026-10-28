// =====================================================================
// FASE G1.4 - GARUDA PUBSUB / GEN_EVENT TEST SUITE
// Memverifikasi pubsub.hpp setara Erlang/OTP gen_event & Phoenix PubSub
// =====================================================================
#include "../include/garuda/beam/otp/pubsub.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <atomic>
#include <vector>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

// Helper: Subscriber process state
struct SubState {
    std::atomic<int> count{0};
    std::vector<Term> received_messages;
};

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G1.4 - GARUDA PUBSUB TESTS\n";
    std::cout << "========================================\n\n";

    // =================================================================
    // Test 1: Subscribe & Publish (1 subscriber)
    // =================================================================
    std::cout << "[Test 1] Subscribe & Publish...\n";
    {
        Runtime rt;
        PubSub ps(rt);

        auto state = std::make_shared<SubState>();

        Pid sub1 = rt.spawn([state](Process& p) -> Status {
            while (true) {
                auto m = p.receive();
                if (!m) return Status::Wait;
                const Term& t = m->body;
                // Message format: tuple({"pubsub_msg", topic, message})
                if (tuple_size(t) == 3 && is_atom(*tuple_at(t, 0), "pubsub_msg")) {
                    state->count.fetch_add(1);
                    state->received_messages.push_back(*tuple_at(t, 2));
                }
            }
        });

        rt.run_until_idle();

        ps.subscribe("news", sub1);
        CHECK(ps.subscriber_count("news") == 1, "Subscriber count news harus 1");

        std::size_t n = ps.publish("news", atom("headline_1"));
        CHECK(n == 1, "Publish harus mengembalikan 1 broadcast count");

        rt.run_until_idle();

        CHECK(state->count.load() == 1, "Subscriber 1 harus menerima 1 pesan");
        CHECK(is_atom(state->received_messages[0], "headline_1"), "Pesan harus headline_1");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 2: Multiple subscribers (3 subscribers)
    // =================================================================
    std::cout << "[Test 2] Multiple subscribers...\n";
    {
        Runtime rt;
        PubSub ps(rt);

        auto s1 = std::make_shared<SubState>();
        auto s2 = std::make_shared<SubState>();
        auto s3 = std::make_shared<SubState>();

        auto make_sub = [](std::shared_ptr<SubState> st) {
            return [st](Process& p) -> Status {
                while (true) {
                    auto m = p.receive();
                    if (!m) return Status::Wait;
                    if (tuple_size(m->body) == 3 && is_atom(*tuple_at(m->body, 0), "pubsub_msg")) {
                        st->count.fetch_add(1);
                        st->received_messages.push_back(*tuple_at(m->body, 2));
                    }
                }
            };
        };

        Pid sub1 = rt.spawn(make_sub(s1));
        Pid sub2 = rt.spawn(make_sub(s2));
        Pid sub3 = rt.spawn(make_sub(s3));
        rt.run_until_idle();

        ps.subscribe("updates", sub1);
        ps.subscribe("updates", sub2);
        ps.subscribe("updates", sub3);

        CHECK(ps.subscriber_count("updates") == 3, "Subscriber count updates harus 3");

        std::size_t broadcast_count = ps.publish("updates", Term(static_cast<std::int64_t>(100)));
        CHECK(broadcast_count == 3, "Broadcast count harus 3");

        rt.run_until_idle();

        CHECK(s1->count.load() == 1, "Sub 1 terima 1 pesan");
        CHECK(s2->count.load() == 1, "Sub 2 terima 1 pesan");
        CHECK(s3->count.load() == 1, "Sub 3 terima 1 pesan");

        const std::int64_t* v1 = s1->received_messages[0].get_if<std::int64_t>();
        CHECK(v1 && *v1 == 100, "Sub 1 message value harus 100");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 3: Unsubscribe
    // =================================================================
    std::cout << "[Test 3] Unsubscribe...\n";
    {
        Runtime rt;
        PubSub ps(rt);

        auto s1 = std::make_shared<SubState>();
        Pid sub1 = rt.spawn([s1](Process& p) -> Status {
            while (true) {
                auto m = p.receive();
                if (!m) return Status::Wait;
                if (tuple_size(m->body) == 3 && is_atom(*tuple_at(m->body, 0), "pubsub_msg")) {
                    s1->count.fetch_add(1);
                }
            }
        });
        rt.run_until_idle();

        ps.subscribe("alerts", sub1);
        CHECK(ps.subscriber_count("alerts") == 1, "Subscriber count alerts awal = 1");

        ps.publish("alerts", atom("msg1"));
        rt.run_until_idle();
        CHECK(s1->count.load() == 1, "Sub 1 terima msg1");

        // Unsubscribe
        ps.unsubscribe("alerts", sub1);
        CHECK(ps.subscriber_count("alerts") == 0, "Subscriber count alerts setelah unsub = 0");

        ps.publish("alerts", atom("msg2"));
        rt.run_until_idle();
        CHECK(s1->count.load() == 1, "Sub 1 TIDAK menerima msg2 (count tetap 1)");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 4: Publish to topic without subscribers (no-op)
    // =================================================================
    std::cout << "[Test 4] Publish to topic without subscribers...\n";
    {
        Runtime rt;
        PubSub ps(rt);

        std::size_t count = ps.publish("empty_topic", atom("ignored"));
        CHECK(count == 0, "Publish ke empty topic harus mengembalikan 0");
        CHECK(ps.subscriber_count("empty_topic") == 0, "Subscriber count topic kosong = 0");
    }
    std::cout << "PASS\n\n";

    // =================================================================
    // Test 5: Multiple events sequentially
    // =================================================================
    std::cout << "[Test 5] Multiple events sequentially...\n";
    {
        Runtime rt;
        PubSub ps(rt);

        auto s1 = std::make_shared<SubState>();
        Pid sub1 = rt.spawn([s1](Process& p) -> Status {
            while (true) {
                auto m = p.receive();
                if (!m) return Status::Wait;
                if (tuple_size(m->body) == 3 && is_atom(*tuple_at(m->body, 0), "pubsub_msg")) {
                    s1->count.fetch_add(1);
                    s1->received_messages.push_back(*tuple_at(m->body, 2));
                }
            }
        });
        rt.run_until_idle();

        ps.subscribe("chat", sub1);

        for (int i = 1; i <= 5; ++i) {
            ps.publish("chat", Term(static_cast<std::int64_t>(i)));
        }

        rt.run_until_idle();

        CHECK(s1->count.load() == 5, "Sub 1 harus terima 5 pesan");
        for (int i = 0; i < 5; ++i) {
            const std::int64_t* v = s1->received_messages[i].get_if<std::int64_t>();
            CHECK(v && *v == (i + 1), "Pesan berurutan harus sesuai index + 1");
        }
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G1.4 - PUBSUB SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
