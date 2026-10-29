// =====================================================================
// GVM ADMIN CLI — GARUDA VIRTUAL MACHINE
// Command-Line Management Tool for GVM Status, Metrics & Logs
// =====================================================================

#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char** argv) {
    std::string cmd = (argc > 1) ? argv[1] : "help";

    if (cmd == "status") {
        std::cout << "========================================\n";
        std::cout << "  GVM STATUS & HEALTHCHECK\n";
        std::cout << "========================================\n\n";
        std::cout << "--- [/health Endpoint] ---\n";
        int r1 = std::system("curl -s http://127.0.0.1:9002/health");
        std::cout << "\n\n--- [/metrics Summary] ---\n";
        int r2 = std::system("curl -s http://127.0.0.1:9002/metrics");
        std::cout << "\n";
        (void)r1; (void)r2;
    }
    else if (cmd == "log") {
        std::cout << "========================================\n";
        std::cout << "  GVM RECENT JOURNAL LOGS (JSON)\n";
        std::cout << "========================================\n\n";
        int r = std::system("journalctl -u richat-garuda -n 20 --no-pager");
        (void)r;
    }
    else if (cmd == "metrics") {
        std::cout << "========================================\n";
        std::cout << "  GVM PROMETHEUS METRICS\n";
        std::cout << "========================================\n\n";
        int r = std::system("curl -s http://127.0.0.1:9002/metrics");
        std::cout << "\n";
        (void)r;
    }
    else {
        std::cout << "GVM Admin CLI v67.0 — Garuda Virtual Machine Management\n";
        std::cout << "Usage: gvm-admin [command]\n\n";
        std::cout << "Commands:\n";
        std::cout << "  status   Cek kesehatan server & metrik utama\n";
        std::cout << "  log      Tampilkan 20 baris log JSON terbaru dari journalctl\n";
        std::cout << "  metrics  Tampilkan output mentah Prometheus /metrics\n";
        std::cout << "  help     Tampilkan bantuan ini\n\n";
    }

    return 0;
}
