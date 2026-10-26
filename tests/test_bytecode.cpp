#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/compiler/bytecode_writer.hpp"
#include "../include/garuda/compiler/bytecode_reader.hpp"
#include "../include/garuda/beam/gvm/actor_runtime.hpp"
#include <cassert>
#include <fstream>
#include <iostream>

using namespace garuda::compiler;
using namespace garuda::beam;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "\n❌ ASSERTION FAILED: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main(int argc, char* argv[]) {
    // Optional sanity check flag
    if (argc > 1 && std::string(argv[1]) == "--sanity-check") {
        std::cout << "Running TEST_ASSERT sanity check...\n";
        TEST_ASSERT(1 == 2, "Sanity Check Failure Verification");
        return 0;
    }

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT 1.5 BYTECODE TEST SUITE (8 ASSERTIONS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Simple arithmetic compile -> write binary .gvm -> read -> run ===
    std::string src1 = "modul m1 fungsi utama() -> 5 + 3 rampung";
    Lexer lex1(src1);
    Parser p1(lex1.tokenize());
    auto ast1 = p1.parse_program();
    CodeGenerator cg1;
    auto out1 = cg1.generate(ast1);

    bool write_ok1 = BytecodeWriter::write(out1, "./test_m1.gvm");
    assert(write_ok1); // Assertion 1

    auto read_out1 = BytecodeReader::read("./test_m1.gvm");
    assert(read_out1.has_value()); // Assertion 2
    assert(read_out1->module_name == "m1"); // Assertion 3

    GvmActorRuntime rt1(read_out1->all_instructions, read_out1->function_table);
    auto res1 = rt1.run_until_done("utama");
    assert(res1.has_value() && *res1->get_if<std::int64_t>() == 8); // Assertion 4
    std::cout << "  ✓ Test 1: Write & Read .gvm arithmetic (5 + 3 = 8) PASS\n";

    // === Test 2: Actor program compile -> write binary .gvm -> read -> run actor ping-pong ===
    std::string src2 = R"(
        modul m2
        fungsi server() ->
            tampa
                {ping, Dari} -> Dari <- {pong, diri}
            rampung
        rampung
        fungsi utama() ->
            S = cakra(fungsi() -> server() rampung)
            S <- {ping, diri}
            tampa
                {pong, ServerPid} -> 42
            rampung
        rampung
    )";
    Lexer lex2(src2);
    Parser p2(lex2.tokenize());
    auto ast2 = p2.parse_program();
    CodeGenerator cg2;
    auto out2 = cg2.generate(ast2);

    bool write_ok2 = BytecodeWriter::write(out2, "./test_m2.gvm");
    assert(write_ok2); // Assertion 5

    auto read_out2 = BytecodeReader::read("./test_m2.gvm");
    assert(read_out2.has_value()); // Assertion 6

    GvmActorRuntime rt2(read_out2->all_instructions, read_out2->function_table);
    auto res2 = rt2.run_until_done("utama");
    assert(res2.has_value() && *res2->get_if<std::int64_t>() == 42); // Assertion 7
    std::cout << "  ✓ Test 2: Write & Read .gvm Actor Ping-Pong PASS\n";

    // === Test 3: Checksum validation test (corrupt 1 byte in file) ===
    std::string corrupt_path = "./test_corrupt.gvm";
    BytecodeWriter::write(out1, corrupt_path);

    // Corrupt a byte in the code section
    std::fstream f(corrupt_path, std::ios::in | std::ios::out | std::ios::binary);
    assert(f.is_open());
    f.seekp(40);
    f.put('X');
    f.close();

    auto corrupt_read = BytecodeReader::read(corrupt_path);
    assert(!corrupt_read.has_value()); // Assertion 8: Corrupted file rejected!
    std::cout << "  ✓ Test 3: Checksum validation rejects corrupted file PASS\n";

    // === Test 4: Magic number validation test ===
    std::string bad_magic_path = "./test_bad_magic.gvm";
    BytecodeWriter::write(out1, bad_magic_path);

    std::fstream f2(bad_magic_path, std::ios::in | std::ios::out | std::ios::binary);
    assert(f2.is_open());
    f2.seekp(0);
    f2.write("BADM", 4);
    f2.close();

    auto bad_magic_read = BytecodeReader::read(bad_magic_path);
    assert(!bad_magic_read.has_value()); // Assertion 9: Invalid magic number rejected!
    std::cout << "  ✓ Test 4: Invalid Magic Number rejected PASS\n";

    std::cout << "\n🎉 ALL 9 ASSERTIONS IN SPRINT 1.5 BYTECODE TEST SUITE PASSED PERFECTLY!\n";
    return 0;
}
