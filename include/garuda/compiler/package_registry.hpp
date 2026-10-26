#pragma once
// Garuda Language Compiler: Package Registry & Dependency Resolution Engine (`package_registry.hpp`)
// Mengimplementasikan Semver Resolution (^1.0.0, ~1.0.0, >=1.0.0 <2.0.0), Dependency Resolution, SHA-256 Checksum Verification, & Local Registry.
#include "../beam/stdlib/crypto.hpp"
#include "../beam/stdlib/json.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::compiler {

namespace fs = std::filesystem;
using namespace garuda::beam::stdlib;

struct SemVer {
    int major = 1;
    int minor = 0;
    int patch = 0;

    static SemVer parse(const std::string& str) {
        SemVer v;
        std::string s = str;
        if (!s.empty() && (s.front() == 'v' || s.front() == 'V')) s.erase(0, 1);

        sscanf_s(s.c_str(), "%d.%d.%d", &v.major, &v.minor, &v.patch);
        return v;
    }

    std::string to_string() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    bool operator==(const SemVer& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }

    bool operator<(const SemVer& o) const {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        return patch < o.patch;
    }

    bool operator<=(const SemVer& o) const { return *this < o || *this == o; }
    bool operator>(const SemVer& o) const { return !(*this <= o); }
    bool operator>=(const SemVer& o) const { return !(*this < o); }
};

struct PackageMeta {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::string checksum;
    std::map<std::string, std::string> dependencies;
};

class SemVerResolver {
public:
    static bool satisfies(const SemVer& version, const std::string& req) {
        if (req == "*" || req == "latest" || req.empty()) return true;

        if (!req.empty() && req.front() == '^') { // Caret ^1.0.0 -> major must match
            SemVer target = SemVer::parse(req.substr(1));
            return version.major == target.major && version >= target;
        }

        if (!req.empty() && req.front() == '~') { // Tilde ~1.0.0 -> major & minor must match
            SemVer target = SemVer::parse(req.substr(1));
            return version.major == target.major && version.minor == target.minor && version >= target;
        }

        if (req.find(".*") != std::string::npos) { // Wildcard 1.0.*
            SemVer target = SemVer::parse(req.substr(0, req.find(".*")));
            return version.major == target.major && version.minor == target.minor;
        }

        if (req.find(">=") != std::string::npos && req.find("<") != std::string::npos) {
            auto pos_lt = req.find("<");
            std::string ge_part = req.substr(2, pos_lt - 2);
            std::string lt_part = req.substr(pos_lt + 1);

            ge_part.erase(0, ge_part.find_first_not_of(" \t"));
            if (!ge_part.empty()) ge_part.erase(ge_part.find_last_not_of(" \t") + 1);
            lt_part.erase(0, lt_part.find_first_not_of(" \t"));
            if (!lt_part.empty()) lt_part.erase(lt_part.find_last_not_of(" \t") + 1);

            SemVer min_v = SemVer::parse(ge_part);
            SemVer max_v = SemVer::parse(lt_part);
            return version >= min_v && version < max_v;
        }

        SemVer target = SemVer::parse(req);
        return version == target;
    }
};

class PackageRegistry {
public:
    PackageRegistry() = default;
    explicit PackageRegistry(std::string cache_dir) : cache_dir_(std::move(cache_dir)) {
        if (!fs::exists(cache_dir_)) {
            fs::create_directories(cache_dir_);
        }
    }

    void publish_package(const PackageMeta& meta) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        registry_[meta.name][meta.version] = meta;
    }

    std::optional<PackageMeta> get_package(const std::string& name, const std::string& version) const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        auto it = registry_.find(name);
        if (it != registry_.end()) {
            auto v_it = it->second.find(version);
            if (v_it != it->second.end()) return v_it->second;
        }
        return std::nullopt;
    }

    std::optional<std::string> resolve_best_version(const std::string& name, const std::string& semver_req) const {
        std::lock_guard<std::recursive_mutex> lock(mu_);

        std::string cache_key = name + "@" + semver_req;
        auto cache_it = resolve_cache_.find(cache_key);
        if (cache_it != resolve_cache_.end()) return cache_it->second;

        auto it = registry_.find(name);
        if (it == registry_.end()) return std::nullopt;

        std::vector<SemVer> candidates;
        for (const auto& [ver_str, meta] : it->second) {
            SemVer v = SemVer::parse(ver_str);
            if (SemVerResolver::satisfies(v, semver_req)) {
                candidates.push_back(v);
            }
        }

        if (candidates.empty()) return std::nullopt;

        std::sort(candidates.begin(), candidates.end());
        std::string res = candidates.back().to_string();
        resolve_cache_[cache_key] = res;
        return res;
    }

    bool resolve_dependencies(const std::string& root_pkg, const std::string& root_ver, std::vector<std::string>& resolved_order) const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        std::map<std::string, std::string> resolved;
        std::set<std::string> visiting;

        std::function<bool(const std::string&, const std::string&)> dfs = [&](const std::string& pkg, const std::string& req) -> bool {
            if (visiting.count(pkg)) {
                std::cout << "⚠️ [GARUDA SEMVER] Circular dependency detected in " << pkg << std::endl;
                return false; // Circular dependency detected!
            }

            auto best_ver = resolve_best_version(pkg, req);
            if (!best_ver.has_value()) {
                std::cout << "⚠️ [GARUDA SEMVER] Version conflict / unresolved dependency for " << pkg << " (" << req << ")" << std::endl;
                return false; // Conflict or unresolvable
            }

            if (resolved.count(pkg)) {
                if (resolved[pkg] != *best_ver) {
                    std::cout << "⚠️ [GARUDA SEMVER] Version conflict for " << pkg << ": " << resolved[pkg] << " vs " << *best_ver << std::endl;
                    return false; // Version conflict!
                }
                return true;
            }

            visiting.insert(pkg);
            resolved[pkg] = *best_ver;

            auto meta_opt = get_package(pkg, *best_ver);
            if (meta_opt.has_value()) {
                for (const auto& [dep_name, dep_req] : meta_opt->dependencies) {
                    if (!dfs(dep_name, dep_req)) return false;
                }
            }

            visiting.erase(pkg);
            resolved_order.push_back(pkg + "@" + *best_ver);
            return true;
        };

        return dfs(root_pkg, root_ver);
    }

    bool install_package(const std::string& name, const std::string& version, const std::string& payload, const std::string& expected_checksum) {
        std::string actual_checksum = Sha256::hash(payload);
        if (actual_checksum != expected_checksum) {
            std::cout << "❌ [GARUDA REGISTRY] SHA-256 Checksum mismatch for " << name << "@" << version << "! Expected: " << expected_checksum << ", Got: " << actual_checksum << std::endl;
            return false; // Tampered or corrupted package!
        }

        fs::path pkg_dir = fs::path(cache_dir_) / name / version;
        fs::create_directories(pkg_dir);

        std::ofstream ofs(pkg_dir / "package.grd");
        if (!ofs.is_open()) return false;
        ofs << payload;

        installed_[name] = version;
        return true;
    }

    bool is_installed(const std::string& name) const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        return installed_.find(name) != installed_.end();
    }

    bool uninstall_package(const std::string& name) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        auto it = installed_.find(name);
        if (it != installed_.end()) {
            fs::remove_all(fs::path(cache_dir_) / name);
            installed_.erase(it);
            return true;
        }
        return false;
    }

    std::vector<PackageMeta> search(const std::string& query) const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        std::vector<PackageMeta> results;
        for (const auto& [pkg_name, ver_map] : registry_) {
            if (pkg_name.find(query) != std::string::npos && !ver_map.empty()) {
                results.push_back(ver_map.rbegin()->second);
            }
        }
        return results;
    }

private:
    std::string cache_dir_ = "./.garuda/packages";
    mutable std::recursive_mutex mu_;
    std::map<std::string, std::map<std::string, PackageMeta>> registry_;
    mutable std::unordered_map<std::string, std::string> resolve_cache_;
    std::map<std::string, std::string> installed_;
};

} // namespace garuda::compiler
