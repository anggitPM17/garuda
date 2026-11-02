// Garuda CLI Tooling (`grd.cpp`) — Fondasi Ekosistem Garuda (CLI -> VS Code -> Web)
#include "garuda/beam/tools/script_engine.hpp"
#include "garuda/beam/io/filesystem.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void print_help() {
    std::cout << "Garuda CLI Tooling (grd) v0.1.0\n";
    std::cout << "Usage: grd <perintah> [argumen]\n\n";
    std::cout << "Perintah:\n";
    std::cout << "  run <file.grd>     Jalankan skrip Garuda\n";
    std::cout << "  test               Jalankan semua test suite\n";
    std::cout << "  new <nama>         Buat proyek Garuda baru\n";
    std::cout << "  --help, -h         Tampilkan bantuan ini\n";
    std::cout << "  --version, -v      Tampilkan versi\n";
}

void print_version() {
    std::cout << "grd v0.1.0 (Garuda v67.0.0)\n";
}

int cmd_run(const std::string& path) {
    if (!fs::exists(path)) {
        std::cerr << "Error: File '" << path << "' tidak ditemukan!\n";
        return 1;
    }
    std::string source = garuda::beam::Fs::read_file(path);
    if (source.empty()) {
        std::cerr << "Error: File '" << path << "' kosong atau gagal dibaca!\n";
        return 1;
    }
    garuda::beam::ScriptEngine engine;
    auto res = engine.eval(source);
    if (!res) {
        std::cerr << "Error: Gagal mengevaluasi skrip '" << path << "'!\n";
        return 1;
    }
    std::cout << "=> " << garuda::beam::to_string(*res) << "\n";
    return 0;
}

int cmd_test() {
    struct TestTarget {
        std::string name;
        std::string exe;
        bool skip = false;
        std::string skip_reason = "";
    };

    std::vector<TestTarget> tests = {
        {"Waktu & Durasi", "garuda_waktu_tests.exe", false, ""},
        {"Matematika", "garuda_matematika_tests.exe", false, ""},
        {"HTTP Client (GET/POST)", "garuda_http_tests.exe", false, ""},
        {"String Module", "garuda_string_grd_tests.exe", false, ""},
        {"Interpolasi String", "garuda_interp_tests.exe", false, ""},
        {"Guard Clause (nalika)", "garuda_guard_tests.exe", false, ""},
        {"Pipe Operator (|>)", "garuda_pipe_tests.exe", false, ""},
        {"Actor Concurrency", "garuda_actor_tests.exe", false, ""},
        {"JSON Encoder/Decoder", "garuda_json_grd_tests.exe", false, ""},
        {"Isolate Binop (KB-001)", "garuda_isolate_binop_tests.exe", true, "KB-001: peta/saring + lambda hang (deferred)"}
    };

    std::cout << "=======================================================\n";
    std::cout << "GARUDA CLI TEST RUNNER (grd test)\n";
    std::cout << "=======================================================\n\n";

    int pass_count = 0;
    int fail_count = 0;
    int skip_count = 0;

    for (const auto& t : tests) {
        if (t.skip) {
            std::cout << "[SKIP] " << t.name << " (" << t.skip_reason << ")\n";
            skip_count++;
            continue;
        }

        std::string cmd = ".\\build\\Release\\" + t.exe;
        if (!fs::exists(".\\build\\Release\\" + t.exe) && fs::exists(".\\Release\\" + t.exe)) {
            cmd = ".\\Release\\" + t.exe;
        }

        int code = std::system(cmd.c_str());
        if (code == 0) {
            std::cout << "[PASS] " << t.name << "\n";
            pass_count++;
        } else {
            std::cerr << "[FAIL] " << t.name << " (Exit Code: " << code << ")\n";
            fail_count++;
        }
    }

    std::cout << "\n=======================================================\n";
    std::cout << "RINGKASAN HASIL TEST: " << pass_count << " PASS | " << fail_count << " FAIL | " << skip_count << " SKIP\n";
    std::cout << "=======================================================\n";

    return fail_count == 0 ? 0 : 1;
}

int cmd_new(const std::string& name) {
    if (name.empty()) {
        std::cerr << "Error: Nama proyek tidak boleh kosong!\n";
        return 1;
    }
    fs::path proj_dir(name);
    if (fs::exists(proj_dir)) {
        std::cerr << "Error: Direktori '" << name << "' sudah ada!\n";
        return 1;
    }

    try {
        fs::create_directories(proj_dir);

        std::ofstream main_file(proj_dir / "utama.grd");
        main_file << "modul Utama\n\n";
        main_file << "fungsi utama() ->\n";
        main_file << "    \"Halo dari proyek " << name << "!\"\n";
        main_file << "rampung\n";
        main_file.close();

        std::ofstream readme_file(proj_dir / "README.md");
        readme_file << "# Proyek " << name << "\n\n";
        readme_file << "Dibuat dengan `grd new`.\n\n";
        readme_file << "## Cara Jalankan:\n";
        readme_file << "```powershell\n";
        readme_file << "grd run " << name << "/utama.grd\n";
        readme_file << "```\n";
        readme_file.close();

        std::cout << "✨ Proyek '" << name << "' berhasil dibuat!\n";
        std::cout << "  File utama: " << name << "/utama.grd\n";
        std::cout << "  Jalankan  : grd run " << name << "/utama.grd\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: Gagal membuat proyek: " << e.what() << "\n";
        return 1;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    std::string cmd = argv[1];
    if (cmd == "--help" || cmd == "-h") {
        print_help();
        return 0;
    }
    if (cmd == "--version" || cmd == "-v") {
        print_version();
        return 0;
    }
    if (cmd == "run") {
        if (argc < 3) {
            std::cerr << "Error: Perintah 'grd run' memerlukan argumen file .grd\n";
            return 1;
        }
        return cmd_run(argv[2]);
    }
    if (cmd == "test") {
        return cmd_test();
    }
    if (cmd == "new") {
        if (argc < 3) {
            std::cerr << "Error: Perintah 'grd new' memerlukan nama proyek!\n";
            return 1;
        }
        return cmd_new(argv[2]);
    }

    std::cerr << "Perintah tidak dikenal: '" << cmd << "'\n";
    std::cerr << "Gunakan 'grd --help' untuk melihat daftar perintah.\n";
    return 1;
}
