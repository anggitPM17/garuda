#include "../include/garuda/jalur_langit/backup_manager.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>

using namespace garuda::jalur_langit;
namespace fs = std::filesystem;
#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    fs::path test_root = fs::temp_directory_path() / "test_backup_manager";
    std::error_code ec;
    fs::remove_all(test_root, ec);

    fs::path source = test_root / "source";
    fs::path backups = test_root / "backups";
    fs::create_directories(source);

    // Buat file dummy
    {
        std::ofstream f1(source / "wal.log");
        f1 << "WAL DATA";
    }
    {
        std::ofstream f2(source / "table.dat");
        f2 << "TABLE DATA";
    }

    std::string backup_path = BackupManager::backup(source.string(), backups.string());
    CHECK(!backup_path.empty(), "Backup harus menghasilkan path");
    CHECK(fs::exists(backup_path), "Folder backup harus ada");
    CHECK(fs::exists(fs::path(backup_path) / "wal.log"), "wal.log harus ter-backup");
    CHECK(fs::exists(fs::path(backup_path) / "table.dat"), "table.dat harus ter-backup");
    std::cout << "PASS: Backup berhasil ke " << backup_path << "\n";

    // Hapus source, lalu restore
    fs::remove_all(source, ec);

    bool restored = BackupManager::restore(backup_path, source.string());
    CHECK(restored, "Restore harus sukses");
    CHECK(fs::exists(source / "wal.log"), "wal.log harus ter-restore");
    CHECK(fs::exists(source / "table.dat"), "table.dat harus ter-restore");
    std::cout << "PASS: Restore berhasil.\n";

    fs::remove_all(test_root, ec);
    std::cout << "BACKUP MANAGER TESTS PASSED!\n";
    return 0;
}
