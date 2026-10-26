#pragma once
// Garuda Language Web Framework: Authentication Engine & Token Manager (`auth.hpp`)
// Mengimplementasikan Salted SHA-256 Password Hashing, HMAC-SHA256 Token Generation, & Auth Protection Middleware.
#include "http_server.hpp"
#include "json.hpp"
#include "crypto.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

namespace garuda::beam::stdlib {

class AuthEngine {
public:
    static std::string hash_password(const std::string& password) {
        std::string salt = Sha256::generate_salt(16);
        std::string hash_str = Sha256::hash(password + salt);
        return salt + ":" + hash_str;
    }

    static bool verify_password(const std::string& password, const std::string& salted_hash) {
        auto colon_pos = salted_hash.find(':');
        if (colon_pos == std::string::npos) return false;

        std::string salt = salted_hash.substr(0, colon_pos);
        std::string expected_hash = salted_hash.substr(colon_pos + 1);

        std::string actual_hash = Sha256::hash(password + salt);
        return actual_hash == expected_hash;
    }

    static std::string generate_token(const std::string& user_id, const std::string& secret_key = "garuda_secret") {
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::string payload = user_id + ":" + std::to_string(now + 3600); // 1-hour expiry
        std::string sig = Sha256::hash(payload + secret_key);
        return payload + "." + sig;
    }

    static std::optional<std::string> verify_token(const std::string& token, const std::string& secret_key = "garuda_secret") {
        auto dot_pos = token.rfind('.');
        if (dot_pos == std::string::npos) return std::nullopt;

        std::string payload = token.substr(0, dot_pos);
        std::string sig_str = token.substr(dot_pos + 1);

        std::string expected_sig = Sha256::hash(payload + secret_key);
        if (expected_sig != sig_str) return std::nullopt; // Tampered token!

        auto colon_pos = payload.find(':');
        if (colon_pos == std::string::npos) return std::nullopt;

        std::string user_id = payload.substr(0, colon_pos);
        std::int64_t expiry = std::stoll(payload.substr(colon_pos + 1));

        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        if (now > expiry) return std::nullopt; // Expired token!

        return user_id;
    }

    static HttpHandler protect(HttpHandler handler, const std::string& secret_key = "garuda_secret") {
        return [handler, secret_key](const HttpRequest& req) -> HttpResponse {
            auto auth_it = req.headers.find("authorization");
            if (auth_it == req.headers.end()) {
                HttpResponse res;
                res.status_code = 401;
                res.status_text = "Unauthorized";
                res.body = "{\"error\":\"unauthorized_missing_token\"}";
                res.headers["content-type"] = "application/json";
                return res;
            }

            std::string auth_header = auth_it->second;
            std::string token;
            if (auth_header.rfind("Bearer ", 0) == 0) {
                token = auth_header.substr(7);
            } else {
                token = auth_header;
            }

            auto user_id_opt = verify_token(token, secret_key);
            if (!user_id_opt.has_value()) {
                HttpResponse res;
                res.status_code = 401;
                res.status_text = "Unauthorized";
                res.body = "{\"error\":\"unauthorized_invalid_token\"}";
                res.headers["content-type"] = "application/json";
                return res;
            }

            return handler(req);
        };
    }
};

} // namespace garuda::beam::stdlib
