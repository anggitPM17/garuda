// =====================================================================
// GARUDA COMPILER — MODULAR CMAKE STRUCTURE & TEST REGISTRY SUITE (5 TESTS)
// =====================================================================

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        return 1; \
    } \
} while(0)

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — MODULAR CMAKE STRUCTURE SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: CMakeLists.txt File Access ===
    std::cout << "[TEST 1/5] Running CMakeLists.txt File Access test...\n";
    std::ifstream ifs("CMakeLists.txt");
    if (!ifs.is_open()) {
        ifs.open("../CMakeLists.txt");
    }
    CHECK(ifs.is_open(), "CMakeLists.txt must be accessible");
    std::string cmake_content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    std::cout << "  ✓ Test 1: CMakeLists.txt File Access PASS\n";

    // === Test 2: Foreach Test Target Loop Integrity ===
    std::cout << "[TEST 2/5] Running Foreach Test Target Loop test...\n";
    CHECK(cmake_content.find("foreach(t ") != std::string::npos, "CMakeLists.txt must contain foreach loop for test targets");
    CHECK(cmake_content.find("garuda_scene_graph_tests") != std::string::npos, "garuda_scene_graph_tests target must be registered");
    std::cout << "  ✓ Test 2: Foreach Test Target Loop PASS\n";

    // === Test 3: CTest add_test Registration ===
    std::cout << "[TEST 3/5] Running CTest add_test Registration test...\n";
    CHECK(cmake_content.find("add_test(NAME") != std::string::npos, "CMakeLists.txt must configure CTest suite");
    std::cout << "  ✓ Test 3: CTest add_test Registration PASS\n";

    // === Test 4: MSVC Link Libraries Configuration ===
    std::cout << "[TEST 4/5] Running MSVC Link Libraries test...\n";
    CHECK(cmake_content.find("ws2_32 bcrypt") != std::string::npos, "MSVC build must link ws2_32 and bcrypt");
    std::cout << "  ✓ Test 4: MSVC Link Libraries PASS\n";

    // === Test 5: Linux Link Libraries Configuration ===
    std::cout << "[TEST 5/5] Running Linux Link Libraries test...\n";
    CHECK(cmake_content.find("pthread dl ssl crypto sqlite3") != std::string::npos, "Linux build must link pthread, dl, ssl, crypto, sqlite3");
    std::cout << "  ✓ Test 5: Linux Link Libraries PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 5 MODULAR CMAKE STRUCTURE TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
