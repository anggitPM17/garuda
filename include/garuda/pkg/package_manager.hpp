#ifndef GARUDA_PKG_PACKAGE_MANAGER_HPP
#define GARUDA_PKG_PACKAGE_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

namespace garuda {
namespace pkg {

struct PackageMetadata {
    std::string name;
    std::string version = "1.0.0";
    std::string description;
    std::string author;
    std::string path;
    bool is_valid = false;
};

class GpmPackageManager {
public:
    static std::vector<PackageMetadata> list_packages(const std::string& packages_dir) {
        std::vector<PackageMetadata> packages;
        fs::path p(packages_dir);

        if (!fs::exists(p) || !fs::is_directory(p)) {
            return packages;
        }

        for (const auto& entry : fs::directory_iterator(p)) {
            if (entry.is_directory()) {
                PackageMetadata meta = verify_package(entry.path().string());
                if (meta.is_valid) {
                    packages.push_back(meta);
                }
            }
        }

        std::sort(packages.begin(), packages.end(), [](const PackageMetadata& a, const PackageMetadata& b) {
            return a.name < b.name;
        });

        return packages;
    }

    static PackageMetadata verify_package(const std::string& package_path) {
        PackageMetadata meta;
        meta.path = package_path;
        meta.name = fs::path(package_path).filename().string();
        meta.is_valid = false;

        fs::path manifest_json = fs::path(package_path) / "garuda.json";
        fs::path manifest_toml = fs::path(package_path) / "grdmix.toml";
        fs::path src_dir = fs::path(package_path) / "src";

        if ((fs::exists(manifest_json) || fs::exists(manifest_toml)) && fs::exists(src_dir)) {
            meta.is_valid = true;
            meta.description = "Garuda Official Verified Package";
            meta.author = "Garuda Core Team";
        }

        return meta;
    }

    static bool install_package(const std::string& packages_dir,
                                const std::string& package_name,
                                const std::string& target_dir) {
        fs::path src_pkg = fs::path(packages_dir) / package_name;
        if (!fs::exists(src_pkg)) {
            std::cerr << "[GpmPackageManager] Error: Package '" << package_name << "' not found in " << packages_dir << std::endl;
            return false;
        }

        fs::path dest_pkg = fs::path(target_dir) / "deps" / package_name;
        try {
            fs::create_directories(dest_pkg);
            fs::copy(src_pkg, dest_pkg, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            std::cout << "📦 [GPM] Successfully installed '" << package_name << "' to " << dest_pkg.string() << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[GpmPackageManager] Error installing package: " << e.what() << std::endl;
            return false;
        }
    }
};

} // namespace pkg
} // namespace garuda

#endif // GARUDA_PKG_PACKAGE_MANAGER_HPP
