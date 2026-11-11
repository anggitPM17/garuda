#include <iostream>
#include <cassert>
#include <vector>
#include <filesystem>

#include "garuda/pkg/package_manager.hpp"

using namespace garuda::pkg;
namespace fs = std::filesystem;

void test_list_and_verify_packages() {
    std::string pkg_dir = "packages";
    if (!fs::exists(pkg_dir)) {
        pkg_dir = "garuda_v67_0_production/garuda_v67.0.0/packages";
    }

    assert(fs::exists(pkg_dir) && "packages directory must exist");

    auto packages = GpmPackageManager::list_packages(pkg_dir);
    assert(packages.size() >= 100 && "Expected at least 100 valid official packages in packages/");

    std::cout << "[PASS] test_list_and_verify_packages (Discovered " << packages.size() << " valid packages)" << std::endl;
}

void test_install_package() {
    std::string pkg_dir = "packages";
    if (!fs::exists(pkg_dir)) {
        pkg_dir = "garuda_v67_0_production/garuda_v67.0.0/packages";
    }

    std::string target_dir = "test_gpm_install_target";
    bool install_ok = GpmPackageManager::install_package(pkg_dir, "garuda_algoritma_bloom_filter", target_dir);
    assert(install_ok && "Package installation failed");
    (void)install_ok;

    fs::path installed_path = fs::path(target_dir) / "deps" / "garuda_algoritma_bloom_filter";
    assert(fs::exists(installed_path) && "Installed package path must exist");

    fs::remove_all(target_dir);
    std::cout << "[PASS] test_install_package" << std::endl;
}

int main() {
    std::cout << "--- RUNNING INTEGRATED PACKAGE MANAGER CLIENT TESTS ---" << std::endl;
    test_list_and_verify_packages();
    test_install_package();
    std::cout << "--- ALL INTEGRATED PACKAGE MANAGER CLIENT TESTS PASSED 100% ---" << std::endl;
    return 0;
}
