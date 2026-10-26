#include "../include/garuda/compiler/lsp_engine.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::compiler;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — SPRINT B.2 LANGUAGE SERVER PROTOCOL (LSP) TEST SUITE (10 TESTS)\n";
    std::cout << "=======================================================\n\n";

    LspEngine lsp;

    // === Test 1: LSP Initialize Request ===
    std::cout << "[TEST 1/10] Running LSP Initialize Request test...\n";
    std::string req1 = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{}}";
    std::string resp1 = lsp.handle_request(req1);
    assert(resp1.find("\"capabilities\"") != std::string::npos);
    assert(resp1.find("\"completionProvider\"") != std::string::npos);
    std::cout << "  ✓ Test 1: LSP Initialize Request PASS\n";

    // === Test 2: LSP Diagnostics (Unbound Variable Error) ===
    std::cout << "[TEST 2/10] Running LSP Diagnostics (Unbound Variable) test...\n";
    std::string bad_code = "modul m\nfungsi f() ->\n    UnboundX + 10\nrampung";
    std::string diag2 = lsp.process_document_diagnostics("file:///app.grd", bad_code);
    assert(diag2.find("textDocument/publishDiagnostics") != std::string::npos);
    assert(diag2.find("Unbound variable 'UnboundX'") != std::string::npos);
    std::cout << "  ✓ Test 2: LSP Diagnostics (Unbound Variable) PASS\n";

    // === Test 3: LSP didChange Diagnostics ===
    std::cout << "[TEST 3/10] Running LSP didChange Live Diagnostics test...\n";
    std::string req3 = "{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/didChange\",\"params\":[{\"textDocument\":[{\"uri\":\"file:///m.grd\"},{\"text\":\"modul m fungsi f() -> Y rampung\"}]}]}";
    std::string resp3 = lsp.handle_request(req3);
    assert(resp3.find("Unbound variable 'Y'") != std::string::npos);
    std::cout << "  ✓ Test 3: LSP didChange Live Diagnostics PASS\n";

    // === Test 4: LSP Completion Keyword Autocomplete ===
    std::cout << "[TEST 4/10] Running LSP Completion Autocomplete test...\n";
    std::string req4 = "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"textDocument/completion\",\"params\":{}}";
    std::string resp4 = lsp.handle_request(req4);
    assert(resp4.find("\"modul\"") != std::string::npos);
    assert(resp4.find("\"fungsi\"") != std::string::npos);
    assert(resp4.find("\"tampa\"") != std::string::npos);
    std::cout << "  ✓ Test 4: LSP Completion Autocomplete PASS\n";

    // === Test 5: LSP Hover Documentation ===
    std::cout << "[TEST 5/10] Running LSP Hover Documentation test...\n";
    std::string req5 = "{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"textDocument/hover\",\"params\":{}}";
    std::string resp5 = lsp.handle_request(req5);
    assert(resp5.find("\"contents\"") != std::string::npos);
    assert(resp5.find("Garuda Nusantara Keyword") != std::string::npos);
    std::cout << "  ✓ Test 5: LSP Hover Documentation PASS\n";

    // === Test 6: LSP Definition Symbol Resolution ===
    std::cout << "[TEST 6/10] Running LSP Definition Symbol Resolution test...\n";
    std::string req6 = "{\"jsonrpc\":\"2.0\",\"id\":4,\"method\":\"textDocument/definition\",\"params\":{}}";
    std::string resp6 = lsp.handle_request(req6);
    assert(resp6.find("\"uri\"") != std::string::npos);
    assert(resp6.find("\"range\"") != std::string::npos);
    std::cout << "  ✓ Test 6: LSP Definition Symbol Resolution PASS\n";

    // === Test 7: LSP Valid Code Diagnostics Clean Check ===
    std::cout << "[TEST 7/10] Running LSP Valid Code Clean Check test...\n";
    std::string valid_code = "modul m\nfungsi f() -> X = 10, X + 20 rampung";
    std::string diag7 = lsp.process_document_diagnostics("file:///valid.grd", valid_code);
    assert(diag7.find("diagnostics\":[]") != std::string::npos); // 0 errors for valid code!
    std::cout << "  ✓ Test 7: LSP Valid Code Clean Check PASS\n";

    // === Test 8: LSP JSON-RPC 2.0 Formatting ===
    std::cout << "[TEST 8/10] Running LSP JSON-RPC Formatting test...\n";
    assert(resp1.find("\"jsonrpc\":\"2.0\"") != std::string::npos);
    assert(resp4.find("\"id\":2") != std::string::npos);
    std::cout << "  ✓ Test 8: LSP JSON-RPC Formatting PASS\n";

    // === Test 9: LSP Multi-URI Context ===
    std::cout << "[TEST 9/10] Running LSP Multi-URI Context test...\n";
    std::string diag9 = lsp.process_document_diagnostics("file:///lib/math.grd", "modul Math fungsi add(A, B) -> A + B rampung");
    assert(diag9.find("file:///lib/math.grd") != std::string::npos);
    std::cout << "  ✓ Test 9: LSP Multi-URI Context PASS\n";

    // === Test 10: End-to-End LSP Engine Lifecycle ===
    std::cout << "[TEST 10/10] Running End-to-End LSP Engine Lifecycle test...\n";
    std::string init_req = "{\"jsonrpc\":\"2.0\",\"id\":10,\"method\":\"initialize\",\"params\":{}}";
    std::string init_res = lsp.handle_request(init_req);
    assert(init_res.find("\"id\":10") != std::string::npos);
    std::cout << "  ✓ Test 10: End-to-End LSP Engine Lifecycle PASS\n";

    std::cout << "\n🎉 ALL 10 LSP ENGINE TESTS PASSED PERFECTLY!\n";
    return 0;
}
