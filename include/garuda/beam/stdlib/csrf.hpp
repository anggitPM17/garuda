#pragma once
// Garuda Language Web Framework: CSRF Protection Engine & Middleware (`csrf.hpp`)
// Mengimplementasikan CSRF Token Generation & Verification Middleware.
#include "http_server.hpp"
#include "crypto.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace garuda::beam::stdlib {

class CsrfEngine {
public:
    static std::string generate_token() {
        std::string salt = Sha256::generate_salt(16);
        return "csrf_" + Sha256::hash(salt);
    }

    static bool verify_token(const std::string& req_token, const std::string& expected_token) {
        if (req_token.empty() || expected_token.empty()) return false;
        return req_token == expected_token;
    }

    static HttpHandler protect(HttpHandler handler, const std::string& expected_token) {
        return [handler, expected_token](const HttpRequest& req) -> HttpResponse {
            // GET, HEAD, OPTIONS requests do not require CSRF token
            if (req.method == "GET" || req.method == "HEAD" || req.method == "OPTIONS") {
                return handler(req);
            }

            std::string req_token;
            auto header_it = req.headers.find("x-csrf-token");
            if (header_it != req.headers.end()) {
                req_token = header_it->second;
            } else if (req.body.find("_csrf_token=") != std::string::npos) {
                auto pos = req.body.find("_csrf_token=");
                auto end_pos = req.body.find('&', pos);
                req_token = req.body.substr(pos + 12, (end_pos != std::string::npos ? end_pos : req.body.length()) - (pos + 12));
            }

            if (!verify_token(req_token, expected_token)) {
                HttpResponse res;
                res.status_code = 403;
                res.status_text = "Forbidden";
                res.body = "{\"error\":\"csrf_token_invalid_or_missing\"}";
                res.headers["content-type"] = "application/json";
                return res;
            }

            return handler(req);
        };
    }
};

} // namespace garuda::beam::stdlib
