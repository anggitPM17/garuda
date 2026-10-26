#pragma once
// Garuda Standard Library: Enum Module (`enum.hpp`)
// Mengimplementasikan iterasi & enumerasi himpunan data (masing, hitung, jumlah).
#include "../core/term.hpp"

#include <cstdint>
#include <functional>
#include <vector>

namespace garuda::beam::stdlib {

class Enum {
public:
    static void masing(const std::vector<Term>& lst, std::function<void(Term)> f) {
        for (const auto& item : lst) {
            f(item);
        }
    }

    static std::int64_t hitung(const std::vector<Term>& lst, std::function<bool(Term)> pred) {
        std::int64_t count = 0;
        for (const auto& item : lst) {
            if (pred(item)) count++;
        }
        return count;
    }

    static std::int64_t jumlah(const std::vector<Term>& lst) {
        std::int64_t total = 0;
        for (const auto& item : lst) {
            if (auto* i = item.get_if<std::int64_t>()) {
                total += *i;
            }
        }
        return total;
    }
};

} // namespace garuda::beam::stdlib
