#pragma once
// Garuda Standard Library: String Module (`string.hpp`)
// Mengimplementasikan manipulasi string (panjang, gabung, potong, besar, kecil, mengandung, pisah, rapikan).
#include "../core/term.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam::stdlib {

class Str {
public:
    static std::int64_t panjang(const std::string& s) {
        return static_cast<std::int64_t>(s.length());
    }

    static std::string gabung(const std::string& a, const std::string& b) {
        return a + b;
    }

    static std::string gabung_list(const std::vector<std::string>& items, const std::string& delim) {
        std::ostringstream oss;
        for (std::size_t i = 0; i < items.size(); ++i) {
            if (i > 0) oss << delim;
            oss << items[i];
        }
        return oss.str();
    }

    static std::string potong(const std::string& s, std::int64_t start, std::int64_t len) {
        if (start < 0 || static_cast<std::size_t>(start) >= s.length()) return "";
        return s.substr(static_cast<std::size_t>(start), static_cast<std::size_t>(len));
    }

    static std::string besar(const std::string& s) {
        std::string res = s;
        std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::toupper(c); });
        return res;
    }

    static std::string kecil(const std::string& s) {
        std::string res = s;
        std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::tolower(c); });
        return res;
    }

    static bool mengandung(const std::string& s, const std::string& sub) {
        return s.find(sub) != std::string::npos;
    }

    static std::vector<std::string> pisah(const std::string& s, const std::string& delim) {
        std::vector<std::string> tokens;
        std::size_t start = 0, end = 0;
        while ((end = s.find(delim, start)) != std::string::npos) {
            tokens.push_back(s.substr(start, end - start));
            start = end + delim.length();
        }
        tokens.push_back(s.substr(start));
        return tokens;
    }

    static std::string rapikan(const std::string& s) {
        std::size_t first = s.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        std::size_t last = s.find_last_not_of(" \t\n\r");
        return s.substr(first, (last - first + 1));
    }
};

} // namespace garuda::beam::stdlib
