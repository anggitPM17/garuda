#pragma once
// Garuda Production Infrastructure: Input Validation & XSS Sanitizer (`validator.hpp`)
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

namespace garuda::beam {

class Validator {
public:
    static bool is_valid_email(const std::string& email) {
        if (email.empty()) return false;

        auto at_pos = email.find('@');
        if (at_pos == std::string::npos || at_pos == 0) return false;

        // Ensure only one @
        if (email.find('@', at_pos + 1) != std::string::npos) return false;

        std::string domain = email.substr(at_pos + 1);
        if (domain.empty() || domain.front() == '.' || domain.back() == '.') return false;

        auto dot_pos = domain.find('.');
        if (dot_pos == std::string::npos || dot_pos == 0) return false;

        // Ensure valid characters in username & domain
        for (char c : email) {
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\0') return false;
        }

        return true;
    }

    static bool is_safe_string(const std::string& input) {
        for (char c : input) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (uc == '\0' || uc == '\r' || uc == '\n' || uc == '\t' || uc < 32) {
                return false;
            }
        }
        return true;
    }

    static std::string sanitize_html(const std::string& input) {
        std::string res;
        res.reserve(input.length() * 2);

        for (char c : input) {
            switch (c) {
                case '<':  res += "&lt;"; break;
                case '>':  res += "&gt;"; break;
                case '&':  res += "&amp;"; break;
                case '"':  res += "&quot;"; break;
                case '\'': res += "&#39;"; break;
                default:   res += c; break;
            }
        }

        return res;
    }
};

} // namespace garuda::beam
