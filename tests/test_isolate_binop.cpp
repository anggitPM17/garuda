// =====================================================================
// BINARY OP + FUNCTION CALL REGISTER ISOLATION TEST (TESTS A THROUGH J2)
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
    } else { \
        std::cout << "[PASS] " << msg << "\n"; \
    } \
} while(0)

int main() {
    garuda::beam::ScriptEngine engine;

    // Test A: 1 + 2
    std::string codeA = "modul TestA\nfungsi utama() -> 1 + 2\nrampung";
    auto resA = engine.eval(codeA);
    auto* iA = resA ? resA->get_if<std::int64_t>() : nullptr;
    CHECK(iA != nullptr && *iA == 3, "Test A: 1 + 2 == 3");

    // Test B: Json.decode_int("42") + 1
    std::string codeB = "modul TestB\nimpor \"std/json.grd\"\nfungsi utama() -> Json.decode_int(\"42\") + 1\nrampung";
    auto resB = engine.eval(codeB);
    auto* iB = resB ? resB->get_if<std::int64_t>() : nullptr;
    CHECK(iB != nullptr && *iB == 43, "Test B: Json.decode_int(\"42\") + 1 == 43");

    // Test C: 1 + Json.decode_int("42")
    std::string codeC = "modul TestC\nimpor \"std/json.grd\"\nfungsi utama() -> 1 + Json.decode_int(\"42\")\nrampung";
    auto resC = engine.eval(codeC);
    auto* iC = resC ? resC->get_if<std::int64_t>() : nullptr;
    CHECK(iC != nullptr && *iC == 43, "Test C: 1 + Json.decode_int(\"42\") == 43");

    // Test D: Json.decode_int("42") + Json.decode_int("8")
    std::string codeD = "modul TestD\nimpor \"std/json.grd\"\nfungsi utama() -> Json.decode_int(\"42\") + Json.decode_int(\"8\")\nrampung";
    auto resD = engine.eval(codeD);
    auto* iD = resD ? resD->get_if<std::int64_t>() : nullptr;
    CHECK(iD != nullptr && *iD == 50, "Test D: Json.decode_int(\"42\") + Json.decode_int(\"8\") == 50");

    // Test E: Json.decode_int("42") + Json.decode_int("8") + Json.decode_int("0")
    std::string codeE = "modul TestE\nimpor \"std/json.grd\"\nfungsi utama() -> Json.decode_int(\"42\") + Json.decode_int(\"8\") + Json.decode_int(\"0\")\nrampung";
    auto resE = engine.eval(codeE);
    auto* iE = resE ? resE->get_if<std::int64_t>() : nullptr;
    CHECK(iE != nullptr && *iE == 50, "Test E: Json.decode_int(\"42\") + Json.decode_int(\"8\") + Json.decode_int(\"0\") == 50");

    // Test F: Json.encode_int(42)
    std::string codeF = "modul TestF\nimpor \"std/json.grd\"\nfungsi utama() -> Json.encode_int(42)\nrampung";
    auto resF = engine.eval(codeF);
    auto* sF = resF ? resF->get_if<std::string>() : nullptr;
    CHECK(sF != nullptr && *sF == "42", "Test F: Json.encode_int(42) == \"42\"");

    // Test G: x = Json.decode_int("42") lalu x
    std::string codeG = "modul TestG\nimpor \"std/json.grd\"\nfungsi utama() ->\n x = Json.decode_int(\"42\")\n x\nrampung";
    auto resG = engine.eval(codeG);
    auto* iG = resG ? resG->get_if<std::int64_t>() : nullptr;
    CHECK(iG != nullptr && *iG == 42, "Test G: x = Json.decode_int(\"42\"); x == 42");

    // Test H: Json.decode_int("42")
    std::string codeH = "modul TestH\nimpor \"std/json.grd\"\nfungsi utama() -> Json.decode_int(\"42\")\nrampung";
    auto resH = engine.eval(codeH);
    auto* iH = resH ? resH->get_if<std::int64_t>() : nullptr;
    CHECK(iH != nullptr && *iH == 42, "Test H: Json.decode_int(\"42\") == 42");

    // Test I: (1 + 2) + Json.decode_int("42")
    std::string codeI = "modul TestI\nimpor \"std/json.grd\"\nfungsi utama() -> (1 + 2) + Json.decode_int(\"42\")\nrampung";
    auto resI = engine.eval(codeI);
    auto* iI = resI ? resI->get_if<std::int64_t>() : nullptr;
    CHECK(iI != nullptr && *iI == 45, "Test I: (1 + 2) + Json.decode_int(\"42\") == 45");

    // Test J1: f(x, x + 1) non-tail call parameter overlap protection
    std::string codeJ1 = R"(
        modul TestJ1
        fungsi tambah(a, b) ->
            a + b
        rampung
        fungsi utama() ->
            x = 10
            y = tambah(x, x + 1)
            y
        rampung
    )";
    auto resJ1 = engine.eval(codeJ1);
    auto* iJ1 = resJ1 ? resJ1->get_if<std::int64_t>() : nullptr;
    CHECK(iJ1 != nullptr && *iJ1 == 21, "Test J1: non-tail f(x, x + 1) overlap protection == 21");

    // Test J2: f(x, x + 1) tail call parameter overlap protection
    std::string codeJ2 = R"(
        modul TestJ2
        fungsi tambah(a, b) ->
            a + b
        rampung
        fungsi utama() ->
            x = 10
            tambah(x, x + 1)
        rampung
    )";
    auto resJ2 = engine.eval(codeJ2);
    auto* iJ2 = resJ2 ? resJ2->get_if<std::int64_t>() : nullptr;
    CHECK(iJ2 != nullptr && *iJ2 == 21, "Test J2: tail f(x, x + 1) overlap protection == 21");

    return 0;
}
