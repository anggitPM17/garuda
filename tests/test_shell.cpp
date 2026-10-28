// =====================================================================
// FASE G6.1 - GARUDA SHELL TESTS
// Memverifikasi shell.hpp setara erl shell
// =====================================================================
#include "../include/garuda/beam/shell/shell.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace garuda::beam;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE G6.1 - GARUDA SHELL TESTS\n";
    std::cout << "========================================\n\n";

    Shell sh;
    sh.start();
    CHECK(sh.is_running(), "Shell harus berstatus running");

    // Test 1: Register & Execute Custom Command
    std::cout << "[Test 1] Register & Execute Custom Command...\n";
    {
        sh.register_command("ping", "Ping command", [](const std::vector<std::string>& args) {
            std::string target = args.empty() ? "localhost" : args[0];
            return "PONG " + target;
        });

        auto res1 = sh.execute("ping 127.0.0.1");
        CHECK(res1.success, "Execute ping 127.0.0.1 harus success");
        CHECK(res1.output == "PONG 127.0.0.1", "Output harus 'PONG 127.0.0.1'");
    }
    std::cout << "PASS\n\n";

    // Test 2: Built-in Commands (help, version, echo)
    std::cout << "[Test 2] Built-in Commands...\n";
    {
        auto res_ver = sh.execute("version");
        CHECK(res_ver.success, "Execute version success");
        CHECK(res_ver.output == "Garuda BEAM v67.0.0", "Version output harus 'Garuda BEAM v67.0.0'");

        auto res_echo = sh.execute("echo Hello Garuda Shell");
        CHECK(res_echo.success, "Execute echo success");
        CHECK(res_echo.output == "Hello Garuda Shell", "Echo output harus 'Hello Garuda Shell'");

        auto res_help = sh.execute("help");
        CHECK(res_help.success, "Execute help success");
        CHECK(res_help.output.find("Garuda GVM Shell Commands:") != std::string::npos, "Help output valid");
    }
    std::cout << "PASS\n\n";

    // Test 3: Unknown Command
    std::cout << "[Test 3] Unknown Command...\n";
    {
        auto res_unknown = sh.execute("invalid_command_xyz");
        CHECK(!res_unknown.success, "Unknown command harus return success = false");
        CHECK(res_unknown.error.find("Unknown command") != std::string::npos, "Error text valid");
    }
    std::cout << "PASS\n\n";

    // Test 4: Tokenize with Quotes
    std::cout << "[Test 4] Tokenize with Quotes...\n";
    {
        auto tokens = Shell::tokenize("echo \"hello world\" 123");
        CHECK(tokens.size() == 3, "Tokens size harus 3");
        CHECK(tokens[0] == "echo", "Token 0 = echo");
        CHECK(tokens[1] == "hello world", "Token 1 = hello world (tanpa kutip)");
        CHECK(tokens[2] == "123", "Token 2 = 123");
    }
    std::cout << "PASS\n\n";

    // Test 5: History Tracking & Clear
    std::cout << "[Test 5] History Tracking & Clear...\n";
    {
        auto hist = sh.history();
        CHECK(hist.size() >= 4, "History size harus >= 4");

        sh.execute("clear");
        CHECK(sh.history().empty(), "History harus kosong setelah 'clear'");
    }
    std::cout << "PASS\n\n";

    sh.stop();
    CHECK(!sh.is_running(), "Shell harus berstatus stopped");

    std::cout << "========================================\n";
    std::cout << "FASE G6.1 - SHELL SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
