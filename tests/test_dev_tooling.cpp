// =====================================================================
// GARUDA COMPILER — FASE 18 DEVELOPER TOOLING SUITE (4 TESTS)
// =====================================================================

#include "../include/garuda/compiler/dev_tooling.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace garuda::compiler;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        return 1; \
    } \
} while(0)

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 18 DEVELOPER TOOLING SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Documentation Generator (GrdDocGenerator) ===
    std::cout << "[TEST 1/4] Running Documentation Generator test...\n";
    std::string src1 = R"(
        # @doc Modul Kalkulator terdistribusi murni Garuda.
        modul Kalkulator

        # @doc Menambahkan dua bilangan bulat.
        fungsi tambah(A, B) ->
            A + B
        rampung

        # @doc Mengalikan dua bilangan bulat.
        fungsi kali(X, Y) ->
            X * Y
        rampung
    )";

    GrdDocGenerator doc_gen;
    auto doc_info = doc_gen.extract_doc(src1);
    CHECK(doc_info.name == "Kalkulator", "Doc module name must be Kalkulator");
    CHECK(doc_info.functions.size() == 2, "Doc must extract 2 functions");
    CHECK(doc_info.functions[0].name == "tambah", "Function 1 name must be tambah");

    std::string md_doc = doc_gen.generate_markdown(doc_info);
    CHECK(md_doc.find("# Modul `Kalkulator`") != std::string::npos, "Markdown doc must contain module heading");
    CHECK(md_doc.find("`fungsi tambah(A, B)`") != std::string::npos, "Markdown doc must contain function signature");

    std::string html_doc = doc_gen.generate_html(doc_info);
    CHECK(html_doc.find("<title>Dokumentasi Modul Kalkulator</title>") != std::string::npos, "HTML doc must contain title tag");
    std::cout << "  ✓ Test 1: Documentation Generator PASS\n";

    // === Test 2: Formatter & Linter Interop ===
    std::cout << "[TEST 2/4] Running Formatter & Linter test...\n";
    std::string src2 = R"(
        modul TesTooling
        fungsi hitung(x, y) ->
            res = x + y
            res
        rampung
    )";

    Lexer lexer(src2);
    Parser parser(lexer.tokenize());
    auto ast = parser.parse_program();

    Formatter fmt;
    std::string formatted = fmt.format(ast);
    CHECK(formatted.find("modul TesTooling") != std::string::npos, "Formatted code must preserve module name");
    CHECK(formatted.find("fungsi hitung(x, y)") != std::string::npos, "Formatted code must preserve function decl");

    Linter linter;
    auto warnings = linter.lint(ast);
    CHECK(warnings.empty(), "Valid snake_case code must produce 0 linter warnings");
    std::cout << "  ✓ Test 2: Formatter & Linter PASS\n";

    // === Test 3: Interactive REPL Engine (GrdReplEngine) ===
    std::cout << "[TEST 3/4] Running Interactive REPL Engine test...\n";
    GrdReplEngine repl;

    std::string r1 = repl.eval_line("10 + 20");
    CHECK(r1 == "30", "REPL evaluation 10 + 20 must return 30");

    std::string r2 = repl.eval_line(":help");
    CHECK(r2.find("Perintah REPL Garuda:") != std::string::npos, "REPL :help command must return help text");

    std::string r3 = repl.eval_line(":vars");
    CHECK(r3.find("Riwayat Evaluasi REPL") != std::string::npos, "REPL :vars command must return history");

    std::string r4 = repl.eval_line(":clear");
    CHECK(r4.find("dibersihkan") != std::string::npos, "REPL :clear command must confirm clear");
    std::cout << "  ✓ Test 3: Interactive REPL Engine PASS\n";

    // === Test 4: Code Execution Profiler (GrdProfiler) ===
    std::cout << "[TEST 4/4] Running Code Profiler test...\n";
    GrdProfiler profiler;
    std::string prof_src = "modul P1 fungsi utama() -> 100 + 200 rampung rampung";
    auto prof_metrics = profiler.profile(prof_src);

    CHECK(prof_metrics.elapsed_ms >= 0.0, "Profiler elapsed time must be non-negative");
    CHECK(prof_metrics.instruction_count > 0, "Profiler instruction count must be > 0");
    std::cout << "  ✓ Test 4: Code Profiler PASS\n";

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL 4 DEVELOPER TOOLING TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
