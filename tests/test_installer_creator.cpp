#include <iostream>
#include <cassert>
#include <filesystem>
#include <cstdio>

#include "garuda/compiler/installer_creator.hpp"

using namespace garuda::compiler;
namespace fs = std::filesystem;

void test_create_windows_installer() {
    InstallerConfig cfg;
    cfg.app_name = "TokoGarudaApp";
    cfg.version = "1.0.0";
    cfg.publisher = "Garuda Corp";
    cfg.target_file = "toko_garuda.exe";
    cfg.output_installer_path = "TokoGaruda_Setup.msi";

    bool create_ok = GvmInstallerCreator::create_windows_installer(cfg);
    assert(create_ok && "create_windows_installer failed");
    (void)create_ok;

    assert(fs::exists(cfg.output_installer_path) && "Installer file must exist");
    assert(fs::file_size(cfg.output_installer_path) > 8 && "Installer file size must be > 8 bytes");

    std::remove(cfg.output_installer_path.c_str());
    std::cout << "[PASS] test_create_windows_installer" << std::endl;
}

int main() {
    std::cout << "--- RUNNING GARUDA INSTALLER CREATOR TESTS ---" << std::endl;
    test_create_windows_installer();
    std::cout << "--- ALL GARUDA INSTALLER CREATOR TESTS PASSED 100% ---" << std::endl;
    return 0;
}
