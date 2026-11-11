#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#include "../include/garuda/beam/interop/embedding_api.hpp"

namespace fs = std::filesystem;
using namespace garuda::beam::interop;
using namespace garuda::beam;

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "=======================================================\n";
    std::cout << "🦅 GARUDA CODING STUDIO v2.0 — IDE DESKTOP PESERTA\n";
    std::cout << "=======================================================\n";
    std::cout << " Menyiapkan Lingkungan Koding & Training Engine...\n";

    std::string studio_file = "packages/ide/src/garuda_studio.grd";
    std::ifstream ifs(studio_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + studio_file);
    }

    if (!ifs.is_open()) {
        std::cerr << "⚠️ Error: Berkas packages/ide/src/garuda_studio.grd tidak ditemukan.\n";
        return 1;
    }

    std::string studio_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    if (!host.load_source(studio_code)) {
        std::cerr << "⚠️ Error: Gagal memuat engine GarudaStudioIDE.\n";
        return 1;
    }

    std::cout << "\n✅ Garuda Coding Studio IDE Siap Digunakan Peserta!\n";
    std::cout << "=======================================================\n";
    std::cout << " [1] 📝 Code Editor & Syntax Highlighting : AKTIF\n";
    std::cout << " [2] 🚀 One-Click Runner & Compiler Console: AKTIF\n";
    std::cout << " [3] 🎓 Auto-Grader Training Evaluator    : AKTIF\n";
    std::cout << " [4] 📦 One-Click Standalone .exe Export   : AKTIF\n";
    std::cout << "=======================================================\n\n";

    auto init_res = host.call_function("utama");
    if (init_res.has_value()) {
        std::cout << "\n🎉 Program EditorGaruda.exe Berjalan Sukses dengan Respon: "
                  << to_string(*init_res) << "\n";
    }

    return 0;
}
