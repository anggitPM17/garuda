#pragma once
#include <string>
#include <unordered_map>

namespace garuda::jalur_langit {
    enum class Permission { READ, WRITE, ADMIN };

    class AuthManager {
    public:
        void grant(const std::string& user, const std::string& resource, Permission perm) {
            permissions_[user + ":" + resource] = perm;
        }

        void revoke(const std::string& user, const std::string& resource) {
            permissions_.erase(user + ":" + resource);
        }

        bool check_permission(const std::string& user, const std::string& resource, Permission perm) const {
            auto it = permissions_.find(user + ":" + resource);
            if (it == permissions_.end()) return false;
            return static_cast<int>(it->second) >= static_cast<int>(perm);
        }

    private:
        std::unordered_map<std::string, Permission> permissions_;
    };
}
