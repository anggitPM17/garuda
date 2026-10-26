#pragma once
// Garuda BEAM-core: Crypto, Hashing, Base64, and JWT Module
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

namespace garuda::beam {

class Crypto {
public:
    static std::string base64_encode(const std::string& in) {
        static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        int val = 0, valb = -6;
        for (uint8_t c : in) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back(b64_table[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) out.push_back(b64_table[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return out;
    }

    static std::string base64url_encode(const std::string& in) {
        std::string b64 = base64_encode(in);
        for (char& c : b64) {
            if (c == '+') c = '-';
            else if (c == '/') c = '_';
        }
        while (!b64.empty() && b64.back() == '=') b64.pop_back();
        return b64;
    }

    static std::string base64_decode(const std::string& in) {
        static const std::string b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) T[b64_table[i]] = i;
        T['-'] = 62; T['_'] = 63;

        int val = 0, valb = -8;
        for (uint8_t c : in) {
            if (T[c] == -1) continue;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                out.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return out;
    }

    // Fast Lightweight Hash (FNV-1a 64-bit string digest for tokens & keys)
    static std::string fnv1a_hash(const std::string& str) {
        uint64_t hash = 14695981039346656037ULL;
        for (char c : str) {
            hash ^= static_cast<uint8_t>(c);
            hash *= 1099511628211ULL;
        }
        std::ostringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return ss.str();
    }

    static std::string generate_uuid(const std::string& prefix = "id_") {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;
        uint64_t r1 = dis(gen);
        uint64_t r2 = dis(gen);
        std::ostringstream ss;
        ss << prefix << std::hex << r1 << std::hex << r2;
        return ss.str();
    }

    static std::string generate_otp() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<int> dis(100000, 999999);
        return std::to_string(dis(gen));
    }

    // JWT Token Generator
    static std::string generate_jwt(const std::string& user_id, const std::string& secret) {
        std::string header = base64url_encode("{\"alg\":\"HS256\",\"typ\":\"JWT\"}");
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        auto exp = now + 86400 * 30; // 30 hari

        std::ostringstream payload_ss;
        payload_ss << "{\"uid\":\"" << user_id << "\",\"iat\":" << now << ",\"exp\":" << exp << "}";
        std::string payload = base64url_encode(payload_ss.str());

        std::string signature = base64url_encode(fnv1a_hash(header + "." + payload + "." + secret));
        return header + "." + payload + "." + signature;
    }

    static bool verify_jwt(const std::string& token, const std::string& secret, std::string& out_user_id) {
        auto p1 = token.find('.');
        auto p2 = token.rfind('.');
        if (p1 == std::string::npos || p2 == std::string::npos || p1 == p2) return false;

        std::string header = token.substr(0, p1);
        std::string payload = token.substr(p1 + 1, p2 - (p1 + 1));
        std::string sig = token.substr(p2 + 1);

        std::string expected_sig = base64url_encode(fnv1a_hash(header + "." + payload + "." + secret));
        if (sig != expected_sig) return false;

        std::string decoded_payload = base64_decode(payload);
        auto uid_pos = decoded_payload.find("\"uid\":\"");
        if (uid_pos != std::string::npos) {
            auto uid_end = decoded_payload.find("\"", uid_pos + 7);
            if (uid_end != std::string::npos) {
                out_user_id = decoded_payload.substr(uid_pos + 7, uid_end - (uid_pos + 7));
                return true;
            }
        }
        return false;
    }
};

} // namespace garuda::beam
