#pragma once
// Garuda Standard Library: Map Module (`map.hpp`)
// Mengimplementasikan kamus kata/pasangan kunci-nilai (baru, ambil, taruh, hapus, kunci, nilai, panjang).
#include "../core/term.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace garuda::beam::stdlib {

class Map {
public:
    static std::map<std::string, Term> baru() {
        return {};
    }

    static std::optional<Term> ambil(const std::map<std::string, Term>& m, const std::string& key) {
        auto it = m.find(key);
        if (it != m.end()) return it->second;
        return std::nullopt;
    }

    static std::map<std::string, Term> taruh(const std::map<std::string, Term>& m, const std::string& key, Term val) {
        std::map<std::string, Term> res = m;
        res[key] = std::move(val);
        return res;
    }

    static std::map<std::string, Term> hapus(const std::map<std::string, Term>& m, const std::string& key) {
        std::map<std::string, Term> res = m;
        res.erase(key);
        return res;
    }

    static std::vector<std::string> kunci(const std::map<std::string, Term>& m) {
        std::vector<std::string> res;
        for (const auto& [k, v] : m) {
            res.push_back(k);
        }
        return res;
    }

    static std::vector<Term> nilai(const std::map<std::string, Term>& m) {
        std::vector<Term> res;
        for (const auto& [k, v] : m) {
            res.push_back(v);
        }
        return res;
    }

    static std::int64_t panjang(const std::map<std::string, Term>& m) {
        return static_cast<std::int64_t>(m.size());
    }
};

} // namespace garuda::beam::stdlib
