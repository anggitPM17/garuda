#pragma once
// Garuda Language Global Parity Engine: Phoenix-Class Web Framework & Channels (`phoenix_framework.hpp`)
#include "global_parity.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <string>

namespace garuda::beam::global {

class PhoenixRouter {
public:
    PhoenixRouter() = default;

    void get(const std::string& path, std::function<std::string()> handler) {
        get_routes_[path] = std::move(handler);
    }

    std::string dispatch(const std::string& path) const {
        auto it = get_routes_.find(path);
        if (it != get_routes_.end() && it->second) {
            return it->second();
        }
        return "404 Not Found";
    }

private:
    std::map<std::string, std::function<std::string()>> get_routes_;
};

} // namespace garuda::beam::global
