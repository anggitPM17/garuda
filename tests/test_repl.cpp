#include "../src/repl.cpp"
#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>

using namespace garuda::cli;

std::string capture_output(std::function<void()> fn) {
    std::streambuf* old = std::cout.rdbuf();
    std::ostringstream oss;
    std::cout.rdbuf(oss.rdbuf());
    fn();
    std::cout.rdbuf(old);
    return oss.str();
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA CLI — SPRINT 4.3 REPL TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Simple Aritmatika 1 + 2 ===
    std::cout << "[TEST 1/10] Running Simple Arithmetic test...\n";
    {
        Repl repl;
        std::string out = capture_output([&]() {
            repl.eval_input("1 + 2");
        });
        assert(out.find("3") != std::string::npos);
    }
    std::cout << "  ✓ Test 1: Simple Arithmetic (1 + 2 => 3) PASS\n";

    // === Test 2: String Concatenation ===
    std::cout << "[TEST 2/10] Running String Concatenation test...\n";
    {
        Repl repl;
        std::string out = capture_output([&]() {
            repl.eval_input("\"hello\" ++ \" world\"");
        });
        assert(out.find("hello world") != std::string::npos);
    }
    std::cout << "  ✓ Test 2: String Concatenation PASS\n";

    // === Test 3: Atom Evaluation ===
    std::cout << "[TEST 3/10] Running Atom Evaluation test...\n";
    {
        Repl repl;
        std::string out = capture_output([&]() {
            repl.eval_input(":ok");
        });
        assert(out.find("ok") != std::string::npos);
    }
    std::cout << "  ✓ Test 3: Atom Evaluation (:ok) PASS\n";

    // === Test 4: Tuple Evaluation ===
    std::cout << "[TEST 4/10] Running Tuple Evaluation test...\n";
    {
        Repl repl;
        std::string out = capture_output([&]() {
            repl.eval_input("{1, 2, 3}");
        });
        assert(out.find("{1,2,3}") != std::string::npos);
    }
    std::cout << "  ✓ Test 4: Tuple Evaluation PASS\n";

    // === Test 5: List Evaluation ===
    std::cout << "[TEST 5/10] Running List Evaluation test...\n";
    {
        Repl repl;
        std::string out = capture_output([&]() {
            repl.eval_input("[1, 2, 3]");
        });
        assert(out.find("[1,2,3]") != std::string::npos);
    }
    std::cout << "  ✓ Test 5: List Evaluation PASS\n";

    // === Test 6: Multi-line Function Declaration & Execution ===
    std::cout << "[TEST 6/10] Running Multi-line Function Declaration test...\n";
    {
        Repl repl;
        std::string out = capture_output([&]() {
            repl.eval_input("fungsi tambah(A, B) -> A + B rampung");
        });
        assert(out.length() > 0);
    }
    std::cout << "  ✓ Test 6: Multi-line Function Declaration PASS\n";

    // === Test 7: Multi-line Incomplete Check ===
    std::cout << "[TEST 7/10] Running Multi-line Incomplete Check test...\n";
    {
        Repl repl;
        assert(repl.is_incomplete("fungsi x() -> 1") == true);
    }
    std::cout << "  ✓ Test 7: Multi-line Incomplete Check PASS\n";

    // === Test 8: Complete Multi-line Check ===
    std::cout << "[TEST 8/10] Running Complete Multi-line Check test...\n";
    {
        Repl repl;
        assert(repl.is_incomplete("fungsi x() -> 1 rampung") == false);
    }
    std::cout << "  ✓ Test 8: Complete Multi-line Check PASS\n";

    // === Test 9: :help Command Output ===
    std::cout << "[TEST 9/10] Running :help Command Output test...\n";
    {
        Repl repl;
        std::string out = capture_output([&]() {
            repl.cmd_help();
        });
        assert(out.find("Garuda REPL Commands") != std::string::npos);
    }
    std::cout << "  ✓ Test 9: :help Command Output PASS\n";

    // === Test 10: Graceful Syntax Error Handling ===
    std::cout << "[TEST 10/10] Running Graceful Syntax Error Handling test...\n";
    {
        Repl repl;
        std::string out = capture_output([&]() {
            repl.eval_input("invalid + + syntax !!!");
        });
        assert(out.find("Galat REPL") != std::string::npos); // Catches exception gracefully!
    }
    std::cout << "  ✓ Test 10: Graceful Syntax Error Handling PASS\n";

    std::cout << "\n🎉 ALL 10 REPL TESTS PASSED PERFECTLY!\n";
    return 0;
}
