#pragma once
// Garuda Desktop Toolchain: Filesystem Abstraction (`filesystem.hpp`)
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace garuda::beam {

namespace fs = std::filesystem;

class Fs {
public:
    static std::string read_file(const std::string& path) {
        try {
            if (!fs::exists(path)) return "";
            std::ifstream ifs(path, std::ios::binary);
            if (!ifs.is_open()) return "";
            return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        } catch (...) {
            return "";
        }
    }

    static bool write_file(const std::string& path, const std::string& content) {
        try {
            fs::path p(path);
            if (p.has_parent_path()) {
                fs::create_directories(p.parent_path());
            }
            std::ofstream ofs(path, std::ios::trunc | std::ios::binary);
            if (!ofs.is_open()) return false;
            ofs << content;
            return ofs.good();
        } catch (...) {
            return false;
        }
    }

    static bool exists(const std::string& path) {
        try {
            return fs::exists(path);
        } catch (...) {
            return false;
        }
    }

    static bool create_dir(const std::string& path) {
        try {
            return fs::create_directories(path);
        } catch (...) {
            return false;
        }
    }

    static bool remove_file(const std::string& path) {
        try {
            if (!fs::exists(path)) return true;
            return fs::remove_all(path) > 0;
        } catch (...) {
            return false;
        }
    }

    static std::vector<std::string> list_dir(const std::string& path) {
        std::vector<std::string> entries;
        try {
            if (!fs::exists(path) || !fs::is_directory(path)) return entries;
            for (const auto& entry : fs::directory_iterator(path)) {
                entries.push_back(entry.path().filename().string());
            }
        } catch (...) {}
        return entries;
    }
};

} // namespace garuda::beam
