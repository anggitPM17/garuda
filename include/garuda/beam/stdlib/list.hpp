#pragma once
// Garuda Standard Library: List Module (`list.hpp`)
// Mengimplementasikan pemrosesan list fungsional (panjang, kepala, ekor, tambah, peta, saring, lipat, urutkan, kosong).
#include "../core/term.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

namespace garuda::beam::stdlib {

class List {
public:
    static std::int64_t panjang(const std::vector<Term>& lst) {
        return static_cast<std::int64_t>(lst.size());
    }

    static Term kepala(const std::vector<Term>& lst) {
        if (lst.empty()) return Term();
        return lst.front();
    }

    static std::vector<Term> ekor(const std::vector<Term>& lst) {
        if (lst.size() <= 1) return {};
        return std::vector<Term>(lst.begin() + 1, lst.end());
    }

    static std::vector<Term> tambah(const std::vector<Term>& lst, Term item) {
        std::vector<Term> res = lst;
        res.push_back(std::move(item));
        return res;
    }

    static std::vector<Term> peta(const std::vector<Term>& lst, std::function<Term(Term)> f) {
        std::vector<Term> res;
        res.reserve(lst.size());
        for (const auto& item : lst) {
            res.push_back(f(item));
        }
        return res;
    }

    static std::vector<Term> saring(const std::vector<Term>& lst, std::function<bool(Term)> pred) {
        std::vector<Term> res;
        for (const auto& item : lst) {
            if (pred(item)) res.push_back(item);
        }
        return res;
    }

    static Term lipat(const std::vector<Term>& lst, Term init, std::function<Term(Term, Term)> f) {
        Term acc = init;
        for (const auto& item : lst) {
            acc = f(item, acc);
        }
        return acc;
    }

    static std::vector<Term> urutkan(const std::vector<Term>& lst) {
        std::vector<Term> res = lst;
        std::sort(res.begin(), res.end(), [](const Term& a, const Term& b) {
            auto* i1 = a.get_if<std::int64_t>();
            auto* i2 = b.get_if<std::int64_t>();
            if (i1 && i2) return *i1 < *i2;
            return to_string(a) < to_string(b);
        });
        return res;
    }

    static bool kosong(const std::vector<Term>& lst) {
        return lst.empty();
    }
};

} // namespace garuda::beam::stdlib
