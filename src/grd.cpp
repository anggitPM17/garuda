// Garuda CLI Tooling (`grd.cpp`) — Fondasi Ekosistem Garuda (CLI -> VS Code -> Web)
#include "garuda/beam/tools/script_engine.hpp"
#include "garuda/beam/io/filesystem.hpp"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void print_help() {
    std::cout << "Garuda CLI Tooling (grd) v0.1.0\n";
    std::cout << "Usage: grd <perintah> [argumen]\n\n";
    std::cout << "Perintah:\n";
    std::cout << "  run <file.grd>       Jalankan skrip Garuda\n";
    std::cout << "  repl                 REPL interaktif (Read-Eval-Print Loop)\n";
    std::cout << "  test                 Jalankan semua test suite C++\n";
    std::cout << "  test-file <file.grd> Jalankan berkas pengujian Garuda\n";
    std::cout << "  new <nama>           Buat proyek Garuda baru\n";
    std::cout << "  --help, -h           Tampilkan bantuan ini\n";
    std::cout << "  --version, -v        Tampilkan versi\n";
}

int cmd_test_file(const std::string& path) {
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
        std::cerr << "Error: Gagal mengevaluasi test file '" << path << "'!\n";
        return 1;
    }
    if (auto* atom = res->get_if<garuda::beam::Atom>()) {
        if (atom->name == "error") {
            return 1;
        }
    }
    std::string result_str = garuda::beam::to_string(*res);
    if (result_str == "error" || result_str == ":error") {
        return 1;
    }
    return 0;
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
        {"HTTP Server", "garuda_http_server_tests.exe", false, ""},
        {"Web Handler Garuda", "garuda_web_handler_tests.exe", false, ""},
        {"String Module", "garuda_string_grd_tests.exe", false, ""},
        {"Map Module", "garuda_map_grd_tests.exe", false, ""},
        {"Tuple Module", "garuda_tuple_grd_tests.exe", false, ""},
        {"Testing Framework", "garuda_test_framework_tests.exe", false, ""},
        {"Interpolasi String", "garuda_interp_tests.exe", false, ""},
        {"Guard Clause (nalika)", "garuda_guard_tests.exe", false, ""},
        {"Pipe Operator (|>)", "garuda_pipe_tests.exe", false, ""},
        {"Actor Concurrency", "garuda_actor_tests.exe", false, ""},
        {"JSON Encoder/Decoder", "garuda_json_grd_tests.exe", false, ""},
        {"Error Handling (coba/tangkap)", "garuda_error_tests.exe", false, ""},
        {"Type Annotation Parser", "garuda_type_annotation_tests.exe", false, ""},
        {"Type Checking Minimal", "garuda_type_check_tests.exe", false, ""},
        {"Sum Types & Pattern Matching", "garuda_sum_type_tests.exe", false, ""},
        {"Isolate Binop (KB-001)", "garuda_isolate_binop_tests.exe", false, ""}
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

int count_open_blocks(const std::string& src) {
    int opens = 0;
    int closes = 0;
    auto count_word = [&](const std::string& word) {
        int n = 0;
        size_t pos = 0;
        while ((pos = src.find(word, pos)) != std::string::npos) {
            bool left_ok = (pos == 0) || !std::isalnum(static_cast<unsigned char>(src[pos - 1]));
            bool right_ok = (pos + word.length() >= src.length()) || !std::isalnum(static_cast<unsigned char>(src[pos + word.length()]));
            if (left_ok && right_ok) n++;
            pos += word.length();
        }
        return n;
    };
    opens += count_word("fungsi");
    opens += count_word("yen");
    opens += count_word("tinjau");
    opens += count_word("cakra");
    opens += count_word("tampa");
    closes += count_word("rampung");
    return opens - closes;
}

int cmd_repl() {
    garuda::beam::ScriptEngine engine;
    std::vector<std::string> history;
    std::string line;

    std::cout << "Garuda REPL v0.1.0 (v67.0.0)\n";
    std::cout << "Ketik :help untuk bantuan, :quit untuk keluar.\n\n";

    while (true) {
        std::cout << "garuda> ";
        if (!std::getline(std::cin, line)) break;

        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);

        if (line == ":quit" || line == ":q") {
            std::cout << "Sampai jumpa!\n";
            break;
        }
        if (line == ":help" || line == ":h") {
            std::cout << "Perintah khusus:\n";
            std::cout << "  :quit, :q     Keluar dari REPL\n";
            std::cout << "  :help, :h     Tampilkan bantuan ini\n";
            std::cout << "  :reset        Reset state engine\n";
            std::cout << "  :history      Tampilkan history input\n";
            std::cout << "  :load <file>  Load file .grd\n";
            std::cout << "\nInput Garuda biasa akan dievaluasi.\n";
            std::cout << "Multi-line: akhiri dengan 'rampung' untuk menutup blok.\n";
            continue;
        }
        if (line == ":reset") {
            engine = garuda::beam::ScriptEngine();
            std::cout << "Engine di-reset.\n";
            continue;
        }
        if (line == ":history") {
            for (size_t i = 0; i < history.size(); ++i) {
                std::cout << "  " << (i + 1) << ": " << history[i] << "\n";
            }
            continue;
        }
        if (line.rfind(":load ", 0) == 0) {
            std::string path = line.substr(6);
            size_t s = path.find_first_not_of(" \t");
            if (s != std::string::npos) path = path.substr(s);
            std::ifstream f(path);
            if (!f.is_open()) {
                std::cerr << "Error: tidak bisa buka file '" << path << "'\n";
                continue;
            }
            std::stringstream ss;
            ss << f.rdbuf();
            std::string src = ss.str();
            auto res = engine.eval(src);
            if (res) {
                std::cout << "=> " << garuda::beam::to_string(*res) << "\n";
            } else {
                std::cout << "=> (tidak ada hasil)\n";
            }
            continue;
        }

        history.push_back(line);

        std::string full_input = line;
        int open_blocks = count_open_blocks(full_input);
        while (open_blocks > 0) {
            std::cout << "     > ";
            std::string next;
            if (!std::getline(std::cin, next)) break;
            full_input += "\n" + next;
            history.push_back(next);
            open_blocks = count_open_blocks(full_input);
        }

        std::string code_to_eval = full_input;
        if (code_to_eval.find("modul ") == std::string::npos) {
            code_to_eval = "modul ReplScript\nfungsi utama() -> " + full_input + "\nrampung";
        }

        auto res = engine.eval(code_to_eval);
        if (res) {
            std::cout << "=> " << garuda::beam::to_string(*res) << "\n";
        } else {
            std::cout << "=> (tidak ada hasil)\n";
        }
    }
    return 0;
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
    if (cmd == "repl") {
        return cmd_repl();
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
    if (cmd == "test-file") {
        if (argc < 3) {
            std::cerr << "Error: Perintah 'grd test-file' memerlukan argumen file .grd\n";
            return 1;
        }
        return cmd_test_file(argv[2]);
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
