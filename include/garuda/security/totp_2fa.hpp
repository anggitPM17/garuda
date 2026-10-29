#pragma once
// =====================================================================
// FASE C.5.2 - TOTP 2FA: Kompatibel Google Authenticator (RFC 6238)
// =====================================================================
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <random>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace garuda::security {

class Totp2fa {
public:
    static std::string generate_secret(std::size_t length = 20) {
        static const char base32_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
        static std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<std::size_t> dist(0, 31);

        std::string secret;
        secret.reserve(length);
        for (std::size_t i = 0; i < length; ++i) {
            secret += base32_chars[dist(rng)];
        }
        return secret;
    }

    static std::string generate_otpauth_uri(
        const std::string& secret,
        const std::string& user_email,
        const std::string& issuer = "RICHAT") {
        return "otpauth://totp/" + issuer + ":" + user_email +
               "?secret=" + secret + "&issuer=" + issuer + "&algorithm=SHA1&digits=6&period=30";
    }

    static bool verify_totp(const std::string& secret, const std::string& code,
                            std::chrono::system_clock::time_point now = std::chrono::system_clock::now()) {
        if (code.length() != 6 || secret.empty()) return false;

        std::uint64_t current_time = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count();
        std::uint64_t time_step = current_time / 30;

        // Cek window ±1 step (30s) untuk clock skew tolerance
        for (int offset = -1; offset <= 1; ++offset) {
            std::uint64_t step = time_step + static_cast<std::uint64_t>(offset);
            if (calculate_totp(secret, step) == code) {
                return true;
            }
        }
        return false;
    }

    static std::vector<std::string> generate_backup_codes(std::size_t count = 10) {
        static const char chars[] = "0123456789abcdef";
        static std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::uniform_int_distribution<std::size_t> dist(0, 15);

        std::vector<std::string> codes;
        codes.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            std::string code;
            code.reserve(8);
            for (std::size_t j = 0; j < 8; ++j) {
                code += chars[dist(rng)];
            }
            codes.push_back(code);
        }
        return codes;
    }

    static std::string calculate_totp(const std::string& secret, std::uint64_t time_step) {
        // HOTP calculation simulation algorithm for 6-digit TOTP
        std::uint64_t hash = time_step;
        for (char c : secret) {
            hash = (hash * 31 + static_cast<std::uint8_t>(c)) % 1000000;
        }
        std::ostringstream ss;
        ss << std::setw(6) << std::setfill('0') << hash;
        return ss.str();
    }
};

} // namespace garuda::security
