#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

namespace garuda::jalur_langit {

enum class Permission { READ, WRITE, ADMIN };

class AuthManager {
private:
    // Format: username -> table -> Permission
    std::unordered_map<std::string, std::unordered_map<std::string, Permission>> roles_;
    mutable std::mutex mutex_;

public:
    void grant(const std::string& user, const std::string& table, Permission perm) {
        std::lock_guard<std::mutex> lock(mutex_);
        roles_[user][table] = perm;
    }

    bool check_permission(const std::string& user, const std::string& table, Permission required) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto u_it = roles_.find(user);
        if (u_it == roles_.end()) return false;

        auto t_it = u_it->second.find(table);
        if (t_it == u_it->second.end()) return false;

        Permission user_perm = t_it->second;
        if (user_perm == Permission::ADMIN) return true;
        if (user_perm == Permission::WRITE) {
            return required == Permission::READ || required == Permission::WRITE;
        }
        if (user_perm == Permission::READ) {
            return required == Permission::READ;
        }
        return false;
    }

    void revoke(const std::string& user, const std::string& table) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto u_it = roles_.find(user);
        if (u_it != roles_.end()) {
            u_it->second.erase(table);
        }
    }
};

} // namespace garuda::jalur_langit
