// =====================================================================
// GARUDA COMPILER — FASE 8 NATIVE TCP SOCKET TEST SUITE (5 TESTS)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 8 NATIVE TCP SOCKET TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    // Test 1: Soket.buka(9876) -> Sid > 0
    std::string code1 = R"(
        modul TestS1
        fungsi utama() ->
            Sid = Soket.buka(9876)
            Soket.tutup(Sid)
            Sid > 0
        rampung
    )";
    auto res1 = engine.eval(code1);
    CHECK(res1.has_value(), "Test 1 must return value");
    auto* i1 = res1->get_if<std::int64_t>();
    auto* a1 = res1->get_if<garuda::beam::Atom>();
    bool pass1 = (i1 != nullptr && *i1 == 1) || (a1 != nullptr && (a1->name == "true" || a1->name == "benar"));
    CHECK(pass1, "Test 1 result must be true/1");
    std::cout << "[Test 1] Soket.buka(9876) -> Sid > 0 [PASS]\n";

    // Test 2: Soket.terima in server loop
    std::string code2 = R"(
        modul TestS2
        fungsi utama() ->
            Sid = Soket.buka(9877)
            Sid
        rampung
    )";
    auto res2 = engine.eval(code2);
    CHECK(res2.has_value(), "Test 2 server socket must open");
    std::int64_t sid2 = *res2->get_if<std::int64_t>();
    CHECK(sid2 > 0, "Test 2 server socket ID must be positive");

    std::int64_t client_cid = 0;
    std::thread accept_thread([&]() {
        garuda::beam::ScriptEngine engine_acc;
        std::string code_acc = "modul Acc fungsi utama() -> Soket.terima(" + std::to_string(sid2) + ") rampung";
        auto r = engine_acc.eval(code_acc);
        if (r && r->get_if<std::int64_t>()) {
            client_cid = *r->get_if<std::int64_t>();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Connect raw client socket
    #ifdef _WIN32
    SOCKET cfd = socket(AF_INET, SOCK_STREAM, 0);
    #else
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    #endif
    sockaddr_in saddr;
    std::memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9877);
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);
    connect(cfd, (struct sockaddr*)&saddr, sizeof(saddr));

    if (accept_thread.joinable()) accept_thread.join();
    CHECK(client_cid > 0, "Test 2 accepted client ID must be positive");
    std::cout << "[Test 2] Soket.terima(Sid) -> Cid = " << client_cid << " [PASS]\n";

    // Test 3: Soket.tulis(Cid, "GARUDA_BEAM_TCP") -> benar
    std::string code3 = R"(
        modul TestS3
        fungsi utama() ->
            Soket.tulis()" + std::to_string(client_cid) + R"(, "GARUDA_BEAM_TCP")
        rampung
    )";
    auto res3 = engine.eval(code3);
    CHECK(res3.has_value(), "Test 3 must return value");
    auto* a3 = res3->get_if<garuda::beam::Atom>();
    CHECK(a3 != nullptr && (a3->name == "benar" || a3->name == "true"), "Test 3 must return benar");
    std::cout << "[Test 3] Soket.tulis(Cid, data): " << a3->name << " [PASS]\n";

    // Read on client socket
    char buf[100] = {0};
    recv(cfd, buf, 100, 0);
    CHECK(std::string(buf) == "GARUDA_BEAM_TCP", "Test 3 client received exact string");

    // Test 4: Soket.baca(Cid, 1024)
    send(cfd, "PING_PONG", 9, 0);

    std::string code4 = R"(
        modul TestS4
        fungsi utama() ->
            Soket.baca()" + std::to_string(client_cid) + R"(, 1024)
        rampung
    )";
    auto res4 = engine.eval(code4);
    CHECK(res4.has_value(), "Test 4 must return value");
    auto* s4 = res4->get_if<std::string>();
    CHECK(s4 != nullptr && *s4 == "PING_PONG", "Test 4 read must be 'PING_PONG'");
    std::cout << "[Test 4] Soket.baca(Cid, 1024): " << *s4 << " [PASS]\n";

    // Test 5: Soket.tutup
    std::string code5 = R"(
        modul TestS5
        fungsi utama() ->
            Soket.tutup()" + std::to_string(client_cid) + R"()
            Soket.tutup()" + std::to_string(sid2) + R"()
        rampung
    )";
    auto res5 = engine.eval(code5);
    CHECK(res5.has_value(), "Test 5 must return value");
    auto* a5 = res5->get_if<garuda::beam::Atom>();
    CHECK(a5 != nullptr && (a5->name == "benar" || a5->name == "true"), "Test 5 must return benar");
    std::cout << "[Test 5] Soket.tutup(Cid) & Soket.tutup(Sid): " << a5->name << " [PASS]\n";

    #ifdef _WIN32
    closesocket(cfd);
    #else
    close(cfd);
    #endif

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 FASE 8 NATIVE TCP SOCKET TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
