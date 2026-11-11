#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#include "garuda/beam/interop/embedding_api.hpp"

using namespace garuda::beam::interop;
using namespace garuda::beam;

void test_garuda_studio_ide_v3() {
    std::string studio_file = "packages/ide/src/garuda_studio.grd";

    std::ifstream ifs(studio_file);
    if (!ifs.is_open()) {
        ifs.open("garuda_v67_0_production/garuda_v67.0.0/" + studio_file);
    }
    assert(ifs.is_open() && "Cannot open packages/ide/src/garuda_studio.grd");

    std::string studio_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(studio_code);
    assert(load_ok && "Failed to load packages/ide/src/garuda_studio.grd via GvmEmbeddingHost");
    (void)load_ok;

    assert(host.has_function("inisialisasi_studio") && "GarudaStudioIDE should have 'inisialisasi_studio' function");
    assert(host.has_function("buka_berkas") && "GarudaStudioIDE should have 'buka_berkas' function");
    assert(host.has_function("jalankan_skrip") && "GarudaStudioIDE should have 'jalankan_skrip' function");
    assert(host.has_function("muat_modul_pelatihan") && "GarudaStudioIDE should have 'muat_modul_pelatihan' function");
    assert(host.has_function("cek_jawaban_peserta") && "GarudaStudioIDE should have 'cek_jawaban_peserta' function");
    assert(host.has_function("format_kode_otomatis") && "GarudaStudioIDE should have 'format_kode_otomatis' function");
    assert(host.has_function("simpan_proyek_peserta") && "GarudaStudioIDE should have 'simpan_proyek_peserta' function");
    assert(host.has_function("muat_contoh_kode_produksi") && "GarudaStudioIDE should have 'muat_contoh_kode_produksi' function");
    assert(host.has_function("tampilkan_laporan_kemajuan_peserta") && "GarudaStudioIDE should have 'tampilkan_laporan_kemajuan_peserta' function");
    assert(host.has_function("ekspor_ke_exe_standalone") && "GarudaStudioIDE should have 'ekspor_ke_exe_standalone' function");
    assert(host.has_function("pasang_paket_gpm") && "GarudaStudioIDE should have 'pasang_paket_gpm' function");
    assert(host.has_function("tampilkan_linter_dan_saran") && "GarudaStudioIDE should have 'tampilkan_linter_dan_saran' function");
    assert(host.has_function("hitung_modul_pelatihan") && "GarudaStudioIDE should have 'hitung_modul_pelatihan' function");

    auto count_res = host.call_function("hitung_modul_pelatihan");
    assert(count_res.has_value() && "hitung_modul_pelatihan should return a value");
    assert(std::get<int64_t>(*count_res) == 4 && "GarudaStudioIDE training curriculum module count should be 4");

    std::cout << "[PASS] test_garuda_studio_ide_v3" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA CODING STUDIO IDE v3.0 TESTS ---" << std::endl;
    test_garuda_studio_ide_v3();
    std::cout << "--- ALL GARUDA CODING STUDIO IDE v3.0 TESTS PASSED 100% ---" << std::endl;
    return 0;
}
