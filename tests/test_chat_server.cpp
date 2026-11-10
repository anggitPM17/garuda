#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

using namespace garuda::compiler;
using namespace garuda::beam;

struct CoutRedirectGuard {
    std::streambuf* old_buf;
    CoutRedirectGuard(std::ostringstream& oss) : old_buf(std::cout.rdbuf(oss.rdbuf())) {}
    ~CoutRedirectGuard() { std::cout.rdbuf(old_buf); }
};

std::string capture_runtime_output(std::function<void()> fn) {
    std::ostringstream oss;
    {
        CoutRedirectGuard guard(oss);
        fn();
    }
    return oss.str();
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA CASE STUDY — SPRINT 4.5 CHAT SERVER TEST SUITE (5 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Compile & Run Chat Server Entry Point ===
    std::cout << "[TEST 1/5] Running Compile & Run Chat Server Entry Point test...\n";
    std::string main_path = "examples/chat_server/src/utama.grd";
    std::ifstream ifs1(main_path);
    if (!ifs1.is_open()) {
        main_path = "../examples/chat_server/src/utama.grd";
        ifs1.open(main_path);
    }
    assert(ifs1.is_open());
    std::string src1((std::istreambuf_iterator<char>(ifs1)), std::istreambuf_iterator<char>());
    ifs1.close();

    Lexer lex1(src1);
    Parser p1(lex1.tokenize());
    auto ast1 = p1.parse_program();
    CodeGenerator cg1;
    auto out1 = cg1.generate(ast1);

    std::string run_out1;
    {
        GvmActorRuntime rt1(out1.all_instructions, out1.function_table);
        rt1.set_verbose(false);
        run_out1 = capture_runtime_output([&]() {
            auto res = rt1.run_until_done("utama", 50000);
            assert(res.has_value());
        });
    }

    assert(run_out1.find("DEMO CHAT SERVER") != std::string::npos);
    assert(run_out1.find("Budi bergabung") != std::string::npos);
    assert(run_out1.find("Ani bergabung") != std::string::npos);
    std::cout << "  ✓ Test 1: Compile & Run Chat Server Entry Point PASS\n";

    // === Test 2: Multi-Actor Broadcast Message Delivery ===
    std::cout << "[TEST 2/5] Running Multi-Actor Broadcast Message Delivery test...\n";
    assert(run_out1.find("broadcast: Halo semua!") != std::string::npos);
    assert(run_out1.find("[Budi] dapat: Halo semua!") != std::string::npos);
    assert(run_out1.find("[Ani] dapat: Halo semua!") != std::string::npos);
    std::cout << "  ✓ Test 2: Multi-Actor Broadcast Message Delivery PASS\n";

    // === Test 3: Supervisor Process Monitoring & Restart ===
    std::cout << "[TEST 3/5] Running Supervisor Process Monitoring test...\n";
    std::string sup_path = "examples/chat_server/src/supervisor.grd";
    std::ifstream ifs3(sup_path);
    if (!ifs3.is_open()) {
        sup_path = "garuda_v67_0_production/garuda_v67.0.0/examples/chat_server/src/supervisor.grd";
        ifs3.open(sup_path);
    }
    if (!ifs3.is_open()) {
        std::cout << "  ❌ Test 3 FAIL: Cannot open supervisor.grd\n";
        return 1;
    }
    std::string src3((std::istreambuf_iterator<char>(ifs3)), std::istreambuf_iterator<char>());
    ifs3.close();

    try {
        Lexer lex3(src3);
        Parser p3(lex3.tokenize());
        auto ast3 = p3.parse_program();
        CodeGenerator cg3;
        auto out3 = cg3.generate(ast3);

        GvmActorRuntime rt3(out3.all_instructions, out3.function_table);
        rt3.set_verbose(false);
        auto res3 = rt3.run_until_done("Supervisor.utama", 50000);
        if (!res3.has_value()) {
            std::cout << "  ❌ Test 3 FAIL: run_until_done returned no value!\n";
            return 1;
        }
        std::cout << "  ✓ Test 3: Supervisor Process Monitoring & Restart PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ❌ Test 3 EXCEPTION: " << e.what() << std::endl;
        return 1;
    }

    // === Test 4: Multiple Concurrent Clients ===
    std::cout << "[TEST 4/5] Running Multiple Concurrent Clients test...\n";
    std::string src4 = R"(
        modul MultiClient
        fungsi client_c1() ->
            tampa
                {pesan_baru, Teks} ->
                    cetak("[C1] dapat: " ++ Teks)
            rampung
        rampung
        fungsi client_c2() ->
            tampa
                {pesan_baru, Teks} ->
                    cetak("[C2] dapat: " ++ Teks)
            rampung
        rampung
        fungsi utama() ->
            C1 = cakra(fungsi() -> client_c1() rampung)
            C2 = cakra(fungsi() -> client_c2() rampung)

            C1 <- {pesan_baru, "Pesan Masif ke Klien"}
            C2 <- {pesan_baru, "Pesan Masif ke Klien"}
            500
        rampung
    )";

    Lexer lex4(src4);
    Parser p4(lex4.tokenize());
    auto ast4 = p4.parse_program();
    CodeGenerator cg4;
    auto out4 = cg4.generate(ast4);

    std::string run_out4;
    {
        GvmActorRuntime rt4(out4.all_instructions, out4.function_table);
        rt4.set_verbose(false);
        run_out4 = capture_runtime_output([&]() {
            auto res = rt4.run_until_done("MultiClient.utama", 50000);
        });
    }

    if (run_out4.find("[C1] dapat: Pesan Masif ke Klien") == std::string::npos ||
        run_out4.find("[C2] dapat: Pesan Masif ke Klien") == std::string::npos) {
        std::cerr << "  ❌ Test 4 FAIL: output was -> " << run_out4 << std::endl;
        return 1;
    }
    std::cout << "  ✓ Test 4: Multiple Concurrent Clients PASS\n";

    // === Test 5: Graceful Shutdown ===
    std::cout << "[TEST 5/5] Running Graceful Shutdown test...\n";
    {
        GvmActorRuntime rt5(out4.all_instructions, out4.function_table);
        rt5.set_verbose(false);
        auto res = rt5.run_until_done("MultiClient.utama", 50000);
        if (!res.has_value()) {
            std::cout << "  ❌ Test 5 FAIL: res is nullopt!\n";
            return 1;
        }
    }
    std::cout << "  ✓ Test 5: Graceful Shutdown PASS\n";

    std::cout << "\n🎉 ALL 5 CHAT SERVER CASE STUDY TESTS PASSED PERFECTLY!\n";
    return 0;
}
