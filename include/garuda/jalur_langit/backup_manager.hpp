#pragma once

#include <string>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <system_error>

namespace garuda::jalur_langit {

namespace fs = std::filesystem;

class BackupManager {
public:
    // Salin semua file dari source_dir ke backup_dir/timestamp_<epoch>
    // Return path folder backup, atau "" jika gagal.
    static std::string backup(const std::string& source_dir, const std::string& backup_dir) {
        try {
            if (!fs::exists(source_dir)) {
                return "";
            }

            auto now = std::chrono::system_clock::now();
            auto epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

            fs::path target = fs::path(backup_dir) / ("timestamp_" + std::to_string(epoch));
            fs::create_directories(target);

            for (const auto& entry : fs::recursive_directory_iterator(source_dir)) {
                if (entry.is_regular_file()) {
                    auto relative = fs::relative(entry.path(), source_dir);
                    fs::path dest = target / relative;
                    fs::create_directories(dest.parent_path());
                    fs::copy_file(entry.path(), dest, fs::copy_options::overwrite_existing);
                }
            }

            return target.string();
        } catch (const std::exception& e) {
            std::cerr << "[BackupManager] Backup error: " << e.what() << "\n";
            return "";
        }
    }

    // Restore: salin semua file dari backup_path ke target_dir (timpa)
    // Return true jika sukses.
    static bool restore(const std::string& backup_path, const std::string& target_dir) {
        try {
            if (!fs::exists(backup_path)) {
                return false;
            }

            fs::create_directories(target_dir);

            for (const auto& entry : fs::recursive_directory_iterator(backup_path)) {
                if (entry.is_regular_file()) {
                    auto relative = fs::relative(entry.path(), backup_path);
                    fs::path dest = fs::path(target_dir) / relative;
                    fs::create_directories(dest.parent_path());
                    fs::copy_file(entry.path(), dest, fs::copy_options::overwrite_existing);
                }
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "[BackupManager] Restore error: " << e.what() << "\n";
            return false;
        }
    }
};

} // namespace garuda::jalur_langit
