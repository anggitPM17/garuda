// =====================================================================
// GVM ADMIN CLI — GARUDA VIRTUAL MACHINE
// Command-Line Management Tool for GVM Status, Metrics & Logs
// =====================================================================

#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>

std::string exec_command(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;
#ifdef _WIN32
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe) return "Error: pipe failed";
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return result;
}

int main(int argc, char** argv) {
    std::string cmd = (argc > 1) ? argv[1] : "help";

    if (cmd == "status") {
        std::cout << "========================================\n";
        std::cout << "  GVM STATUS & HEALTHCHECK\n";
        std::cout << "========================================\n\n";

        std::cout << "--- [/health Endpoint] ---\n";
        std::string health = exec_command("curl -s http://127.0.0.1:9002/health");
        std::cout << (health.empty() ? "(No output)\n" : health);
        if (!health.empty() && health.back() != '\n') std::cout << "\n";

        std::cout << "\n--- [/metrics Summary] ---\n";
        std::string metrics = exec_command("curl -s http://127.0.0.1:9002/metrics");
        std::cout << (metrics.empty() ? "(No output)\n" : metrics);
        if (!metrics.empty() && metrics.back() != '\n') std::cout << "\n";
    }
    else if (cmd == "log") {
        std::cout << "========================================\n";
        std::cout << "  GVM RECENT JOURNAL LOGS (JSON)\n";
        std::cout << "========================================\n\n";
        std::string logs = exec_command("journalctl -u richat-garuda -n 20 --no-pager -o cat");
        std::cout << (logs.empty() ? "(No logs found)\n" : logs);
        if (!logs.empty() && logs.back() != '\n') std::cout << "\n";
    }
    else if (cmd == "metrics") {
        std::cout << "========================================\n";
        std::cout << "  GVM PROMETHEUS METRICS\n";
        std::cout << "========================================\n\n";
        std::string metrics = exec_command("curl -s http://127.0.0.1:9002/metrics");
        std::cout << (metrics.empty() ? "(No output)\n" : metrics);
        if (!metrics.empty() && metrics.back() != '\n') std::cout << "\n";
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
