// =====================================================================
// FASE G2.4 - GARUDA PROCESS GROUP TESTS
// Memverifikasi pg.hpp setara Erlang pg / pg2
// =====================================================================
#include "../include/garuda/beam/dist/pg.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <atomic>
#include <vector>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

struct PgSubState {
    std::atomic<int> count{0};
    std::vector<Term> received_messages;
};

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G2.4 - GARUDA PROCESS GROUP TESTS\n";
    std::cout << "========================================\n\n";

    Runtime rt;
    ProcessGroup pg(rt);

    // Test 1: Join Group
    std::cout << "[Test 1] Join Group...\n";
    {
        Pid p1 = rt.spawn([](Process&) -> Status { return Status::Done; });
        bool ok = pg.join("workers", p1);
        CHECK(ok, "Join group harus berhasil");

        auto members = pg.get_members("workers");
        CHECK(members.size() == 1, "Members size = 1");
        CHECK(members[0] == p1, "Member ID harus p1");
    }
    std::cout << "PASS\n\n";

    // Test 2: Leave Group
    std::cout << "[Test 2] Leave Group...\n";
    {
        Pid p1 = rt.spawn([](Process&) -> Status { return Status::Done; });
        pg.join("temp_group", p1);
        CHECK(pg.get_members("temp_group").size() == 1, "Temp group size = 1");

        bool left = pg.leave("temp_group", p1);
        CHECK(left, "Leave group harus mengembalikan true");
        CHECK(pg.get_members("temp_group").empty(), "Temp group setelah leave = 0 members");
    }
    std::cout << "PASS\n\n";

    // Test 3: Multiple Members
    std::cout << "[Test 3] Multiple Members...\n";
    {
        Pid p1 = rt.spawn([](Process&) -> Status { return Status::Done; });
        Pid p2 = rt.spawn([](Process&) -> Status { return Status::Done; });
        Pid p3 = rt.spawn([](Process&) -> Status { return Status::Done; });

        pg.join("cluster_workers", p1);
        pg.join("cluster_workers", p2);
        pg.join("cluster_workers", p3);

        auto members = pg.get_members("cluster_workers");
        CHECK(members.size() == 3, "Harus ada 3 anggota di cluster_workers");
    }
    std::cout << "PASS\n\n";

    // Test 4: Broadcast Message
    std::cout << "[Test 4] Broadcast Message...\n";
    {
        auto s1 = std::make_shared<PgSubState>();
        auto s2 = std::make_shared<PgSubState>();

        auto make_worker = [](std::shared_ptr<PgSubState> st) {
            return [st](Process& p) -> Status {
                while (true) {
                    auto m = p.receive();
                    if (!m) return Status::Wait;
                    if (tuple_size(m->body) == 3 && is_atom(*tuple_at(m->body, 0), "pg_msg")) {
                        st->count.fetch_add(1);
                        st->received_messages.push_back(*tuple_at(m->body, 2));
                    }
                }
            };
        };

        Pid w1 = rt.spawn(make_worker(s1));
        Pid w2 = rt.spawn(make_worker(s2));
        rt.run_until_idle();

        pg.join("broadcasters", w1);
        pg.join("broadcasters", w2);

        std::size_t count = pg.broadcast("broadcasters", atom("sync_event"));
        CHECK(count == 2, "Broadcast count harus 2");

        rt.run_until_idle();

        CHECK(s1->count.load() == 1, "Worker 1 terima 1 pg_msg");
        CHECK(s2->count.load() == 1, "Worker 2 terima 1 pg_msg");
    }
    std::cout << "PASS\n\n";

    // Test 5: Get Groups Listing
    std::cout << "[Test 5] Get Groups Listing...\n";
    {
        Pid p1 = rt.spawn([](Process&) -> Status { return Status::Done; });
        pg.join("group_a", p1);
        pg.join("group_b", p1);

        auto groups = pg.get_groups();
        CHECK(groups.size() >= 2, "Daftar grup harus mencakup min 2 grup");
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE G2.4 - PROCESS GROUP SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
