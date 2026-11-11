#pragma once
#include <string>
#include <filesystem>

namespace garuda::jalur_langit {
    class BackupManager {
    public:
        static std::string backup(const std::string& src, const std::string& dest) {
            namespace fs = std::filesystem;
            fs::path target = fs::path(dest) / "backup_latest";
            std::error_code ec;
            fs::create_directories(target, ec);
            fs::copy(src, target, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
            return target.string();
        }

        static bool restore(const std::string& src, const std::string& dest) {
            namespace fs = std::filesystem;
            std::error_code ec;
            fs::create_directories(dest, ec);
            fs::copy(src, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
            return !ec;
        }
    };
}
