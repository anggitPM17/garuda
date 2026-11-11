#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#include "../include/garuda/beam/interop/embedding_api.hpp"
#include "../include/garuda/pkg/package_manager.hpp"

namespace fs = std::filesystem;
using namespace garuda::beam::interop;
using namespace garuda::beam;
using namespace garuda::pkg;

int main(int argc, char* argv[]) {
    std::cout << "=======================================================\n";
    std::cout << "🦅 GARUDA CODING STUDIO v2.0 — INTERACTIVE TRAINING HUB\n";
    std::cout << "=======================================================\n";

    std::string target_file = "packages/ide/src/garuda_studio.grd";
    if (argc >= 2) {
        target_file = argv[1];
        std::cout << "📄 Membuka Berkas Kodingan Peserta: " << target_file << "\n";
    }

    std::ifstream ifs(target_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + target_file);
    }

    if (!ifs.is_open()) {
        std::cerr << "⚠️ Error: Berkas " << target_file << " tidak ditemukan.\n";
        return 1;
    }

    std::string code_content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    if (!host.load_source(code_content)) {
        std::cerr << "⚠️ Error: Gagal memuat atau mengompilasi kode sumber.\n";
        return 1;
    }

    std::cout << "\n✅ Lingkungan Koding & Training Engine Siap!\n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << " [1] 🎓 Modul Pelatihan 1 : Dasar Sintaks Garuda\n";
    std::cout << " [2] 🎓 Modul Pelatihan 2 : Aktor Paralel & Concurrency\n";
    std::cout << " [3] 🎓 Modul Pelatihan 3 : REST API Web Server\n";
    std::cout << " [4] 🎓 Modul Pelatihan 4 : Game Engine 3D & ECS\n";
    std::cout << " [5] 📦 Katalog Paket     : Pindai 389 Paket Resmi\n";
    std::cout << "-------------------------------------------------------\n\n";

    auto init_res = host.call_function("utama");
    if (init_res.has_value()) {
        std::cout << "\n🎉 Eksekusi EditorGaruda.exe Berjalan Sukses (Respon: "
                  << to_string(*init_res) << ")\n";
    }

    return 0;
}
