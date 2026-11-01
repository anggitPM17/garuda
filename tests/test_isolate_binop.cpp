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

    // Test K1: f([1,2,3]) pattern matching
    std::string codeK1 = R"(
        modul TestK1
        fungsi f(L) ->
            tinjau L ing
                [] -> "kosong"
                [H | T] -> "ada"
                _ -> "lain"
            rampung
        rampung
        fungsi utama() ->
            f([1, 2, 3])
        rampung
    )";
    auto resK1 = engine.eval(codeK1);
    auto* sK1 = resK1 ? resK1->get_if<std::string>() : nullptr;
    CHECK(sK1 != nullptr && *sK1 == "ada", "Test K1: f([1,2,3]) == \"ada\"");

    // Test K2: f([]) pattern matching
    std::string codeK2 = R"(
        modul TestK2
        fungsi f(L) ->
            tinjau L ing
                [] -> "kosong"
                [H | T] -> "ada"
                _ -> "lain"
            rampung
        rampung
        fungsi utama() ->
            f([])
        rampung
    )";
    auto resK2 = engine.eval(codeK2);
    auto* sK2 = resK2 ? resK2->get_if<std::string>() : nullptr;
    CHECK(sK2 != nullptr && *sK2 == "kosong", "Test K2: f([]) == \"kosong\"");

    // Test K3: g([7,8]) head extraction
    std::string codeK3 = R"(
        modul TestK3
        fungsi g(L) ->
            tinjau L ing
                [H | T] -> H
                _ -> 0
            rampung
        rampung
        fungsi utama() ->
            g([7, 8])
        rampung
    )";
    auto resK3 = engine.eval(codeK3);
    auto* iK3 = resK3 ? resK3->get_if<std::int64_t>() : nullptr;
    CHECK(iK3 != nullptr && *iK3 == 7, "Test K3: g([7,8]) == 7");

    // Test K4: h([1,2,3]) tail match
    std::string codeK4 = R"(
        modul TestK4
        fungsi h(L) ->
            tinjau L ing
                [H | T] -> 2
                _ -> -1
            rampung
        rampung
        fungsi utama() ->
            h([1, 2, 3])
        rampung
    )";
    auto resK4 = engine.eval(codeK4);
    auto* iK4 = resK4 ? resK4->get_if<std::int64_t>() : nullptr;
    CHECK(iK4 != nullptr && *iK4 == 2, "Test K4: h([1,2,3]) == 2");

    // Test K5: x = [1,2,3]; f(x)
    std::string codeK5 = R"(
        modul TestK5
        fungsi f(L) ->
            tinjau L ing
                [] -> "kosong"
                [H | T] -> "ada"
                _ -> "lain"
            rampung
        rampung
        fungsi utama() ->
            x = [1, 2, 3]
            f(x)
        rampung
    )";
    auto resK5 = engine.eval(codeK5);
    auto* sK5 = resK5 ? resK5->get_if<std::string>() : nullptr;
    CHECK(sK5 != nullptr && *sK5 == "ada", "Test K5: x = [1,2,3]; f(x) == \"ada\"");

    // Test K6: cross function call
    std::string codeK6 = R"(
        modul TestK6
        fungsi f(L) ->
            tinjau L ing
                [] -> "kosong"
                [H | T] -> "ada"
                _ -> "lain"
            rampung
        rampung
        fungsi a(L) ->
            f(L)
        rampung
        fungsi utama() ->
            a([1, 2, 3])
        rampung
    )";
    auto resK6 = engine.eval(codeK6);
    auto* sK6 = resK6 ? resK6->get_if<std::string>() : nullptr;
    CHECK(sK6 != nullptr && *sK6 == "ada", "Test K6: a([1,2,3]) == \"ada\"");

    // Test L1: module user WITHOUT import, yen x == 1
    std::string codeL1 = R"(
        modul TestL1
        fungsi p(x) ->
            yen x == 1 -> "a" lain -> "b" rampung
        rampung
        fungsi utama() ->
            p(1)
        rampung
    )";
    auto resL1 = engine.eval(codeL1);
    auto* sL1 = resL1 ? resL1->get_if<std::string>() : nullptr;
    CHECK(sL1 != nullptr && *sL1 == "a", "Test L1: p(1) == \"a\" (no import)");

    // Test L2: module user WITH import "std/json.grd", yen x == 1
    std::string codeL2 = R"(
        modul TestL2
        impor "std/json.grd"
        fungsi p(x) ->
            yen x == 1 -> "a" lain -> "b" rampung
        rampung
        fungsi utama() ->
            p(1)
        rampung
    )";
    auto resL2 = engine.eval(codeL2);
    auto* sL2 = resL2 ? resL2->get_if<std::string>() : nullptr;
    CHECK(sL2 != nullptr && *sL2 == "a", "Test L2: p(1) == \"a\" (with import std/json.grd)");

    // Test L3: module user WITHOUT import, p(2) == "b"
    std::string codeL3 = R"(
        modul TestL3
        fungsi p(x) ->
            yen x == 1 -> "a" lain -> "b" rampung
        rampung
        fungsi utama() ->
            p(2)
        rampung
    )";
    auto resL3 = engine.eval(codeL3);
    auto* sL3 = resL3 ? resL3->get_if<std::string>() : nullptr;
    CHECK(sL3 != nullptr && *sL3 == "b", "Test L3: p(2) == \"b\" (no import)");

    // Test M1: List.panjang([1, 2, 3]) == 3
    std::string codeM1 = R"(
        modul TestM1
        impor "std/json.grd"
        fungsi utama() ->
            List.panjang([1, 2, 3])
        rampung
    )";
    auto resM1 = engine.eval(codeM1);
    auto* iM1 = resM1 ? resM1->get_if<std::int64_t>() : nullptr;
    CHECK(iM1 != nullptr && *iM1 == 3, "Test M1: List.panjang([1,2,3]) == 3");

    // Test M2: List.panjang([]) == 0
    std::string codeM2 = R"(
        modul TestM2
        impor "std/json.grd"
        fungsi utama() ->
            List.panjang([])
        rampung
    )";
    auto resM2 = engine.eval(codeM2);
    auto* iM2 = resM2 ? resM2->get_if<std::int64_t>() : nullptr;
    CHECK(iM2 != nullptr && *iM2 == 0, "Test M2: List.panjang([]) == 0");

    // Test M3: q([1]) == "ada" && q([]) == "kosong"
    std::string codeM3 = R"(
        modul TestM3
        impor "std/json.grd"
        fungsi q(L) ->
            yen List.panjang(L) == 0 -> "kosong" lain -> "ada" rampung
        rampung
        fungsi utama() ->
            q([1])
        rampung
    )";
    auto resM3 = engine.eval(codeM3);
    auto* sM3 = resM3 ? resM3->get_if<std::string>() : nullptr;
    CHECK(sM3 != nullptr && *sM3 == "ada", "Test M3: q([1]) == \"ada\"");

    // Test N1: lp([1, 2, 3], "") == "v,v,v,"
    std::string codeN1 = R"(
        modul TestN1
        fungsi enc(x) -> "v" rampung
        fungsi lp(L, Acc) ->
            tinjau L ing
                [] -> Acc
                [H | T] -> lp(T, Acc ++ enc(H) ++ ",")
                _ -> Acc
            rampung
        rampung
        fungsi utama() ->
            lp([1, 2, 3], "")
        rampung
    )";
    auto resN1 = engine.eval(codeN1);
    auto* sN1 = resN1 ? resN1->get_if<std::string>() : nullptr;
    std::cout << "[N1 val='" << (sN1 ? *sN1 : "null") << "']\n";
    CHECK(sN1 != nullptr && *sN1 == "v,v,v,", "Test N1: lp([1,2,3], \"\") == \"v,v,v,\"");

    // Test N2: lp([1, 2, 3], "") == ",,,"
    std::string codeN2 = R"(
        modul TestN2
        fungsi lp(L, Acc) ->
            tinjau L ing
                [] -> Acc
                [H | T] -> lp(T, Acc ++ ",")
                _ -> Acc
            rampung
        rampung
        fungsi utama() ->
            lp([1, 2, 3], "")
        rampung
    )";
    auto resN2 = engine.eval(codeN2);
    auto* sN2 = resN2 ? resN2->get_if<std::string>() : nullptr;
    std::cout << "[N2 val=',,,']\n";
    CHECK(sN2 != nullptr && *sN2 == ",,,", "Test N2: lp([1,2,3], \"\") == \",,,\"");

    // Test N3: cnt(3, "") == "vvv"
    std::string codeN3 = R"(
        modul TestN3
        fungsi enc(x) -> "v" rampung
        fungsi cnt(n, Acc) ->
            yen n == 0 -> Acc lain -> cnt(n - 1, Acc ++ enc(n)) rampung
        rampung
        fungsi utama() ->
            cnt(3, "")
        rampung
    )";
    auto resN3 = engine.eval(codeN3);
    auto* sN3 = resN3 ? resN3->get_if<std::string>() : nullptr;
    std::cout << "[N3 val='vvv']\n";
    CHECK(sN3 != nullptr && *sN3 == "vvv", "Test N3: cnt(3, \"\") == \"vvv\"");

    // ===== W9/W10: lambda anonim (AnonFunExpr) =====
    // W9: lambda anonim via peta
    std::string codeW9 = R"(
        modul TestW9
        impor "std/json.grd"
        fungsi utama() ->
            List.panjang(List.peta([1, 2, 3], fungsi(X) -> X * 2 rampung))
        rampung
    )";
    auto resW9 = engine.eval(codeW9);
    auto* iW9 = resW9 ? resW9->get_if<std::int64_t>() : nullptr;
    std::cout << "[W9 peta-lambda] " << (iW9 && *iW9 == 3 ? "PASS" : "FAIL") << "\n";
    CHECK(iW9 && *iW9 == 3, "Test W9: List.peta([1,2,3], lambda) len == 3");

    // W10: lambda anonim via lipat
    std::string codeW10 = R"(
        modul TestW10
        impor "std/json.grd"
        fungsi utama() ->
            List.lipat([1, 2, 3], 0, fungsi(A, H) -> A + H rampung)
        rampung
    )";
    auto resW10 = engine.eval(codeW10);
    auto* iW10 = resW10 ? resW10->get_if<std::int64_t>() : nullptr;
    std::cout << "[W10 lipat-lambda] " << (iW10 && *iW10 == 6 ? "PASS" : "FAIL") << "\n";
    CHECK(iW10 && *iW10 == 6, "Test W10: List.lipat([1,2,3], 0, lambda) == 6");

    return 0;
}
