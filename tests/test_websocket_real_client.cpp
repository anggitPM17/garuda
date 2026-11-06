// =====================================================================
// GARUDA COMPILER — FASE 10.8 REAL WEBSOCKET CLIENT C++ LAUNCHER TEST SUITE
// =====================================================================

#include "../include/garuda/beam/tools/script_engine.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

int main() {
    garuda::beam::ScriptEngine engine;

    std::cout << "=======================================================\n";
    std::cout << "GARUDA COMPILER — FASE 10.8 REAL WEBSOCKET CLIENT LAUNCHER\n";
    std::cout << "=======================================================\n\n";

    // Start WebSocket Server on port 9103
    engine.eval("modul TRealClient fungsi utama() -> Server.mulai_ws(9103, nil, 50) rampung");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Invoke Python script
    int ret = std::system("python tests/test_websocket_real_client.py");

    if (ret != 0) {
        std::cout << "\n❌ REAL WEBSOCKET CLIENT TEST SUITE FAILED!\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "🎉 ALL REAL WEBSOCKET CLIENT TESTS PASSED PERFECTLY!\n";
    std::cout << "========================================\n";
    return 0;
}
