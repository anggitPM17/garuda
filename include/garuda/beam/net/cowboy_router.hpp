#pragma once
// Garuda Cowboy-Grade HTTP & WebSocket Router (`cowboy_router.hpp`)
// Mendukung pencocokan rute dinamis dengan parameter URL (:id, :user_id), HTTP verb dispatch, dan middleware.
#include "../io/http_ws_server.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace garuda::beam {

struct RouteMatch {
    bool matched = false;
    std::map<std::string, std::string> bindings;
};

using RouteHandler = std::function<HttpResponse(const HttpRequest& req, const std::map<std::string, std::string>& bindings)>;

struct RouteEntry {
    HttpMethod method;
    std::string pattern;
    RouteHandler handler;
    std::vector<std::string> param_names;
    std::regex regex_pattern;
};

class CowboyRouter {
public:
    void get(const std::string& pattern, RouteHandler handler) {
        add_route(HttpMethod::Get, pattern, std::move(handler));
    }

    void post(const std::string& pattern, RouteHandler handler) {
        add_route(HttpMethod::Post, pattern, std::move(handler));
    }

    void put(const std::string& pattern, RouteHandler handler) {
        add_route(HttpMethod::Put, pattern, std::move(handler));
    }

    void del(const std::string& pattern, RouteHandler handler) {
        add_route(HttpMethod::Delete, pattern, std::move(handler));
    }

    HttpResponse dispatch(const HttpRequest& req) const {
        for (const auto& route : routes_) {
            if (route.method != req.method) continue;

            std::smatch match;
            if (std::regex_match(req.path, match, route.regex_pattern)) {
                std::map<std::string, std::string> bindings;
                for (std::size_t i = 0; i < route.param_names.size(); ++i) {
                    if (i + 1 < match.size()) {
                        bindings[route.param_names[i]] = match[i + 1].str();
                    }
                }
                return route.handler(req, bindings);
            }
        }

        HttpResponse not_found;
        not_found.status_code = 404;
        not_found.status_text = "Not Found";
        not_found.body = "{\"error\":\"route_not_found\"}";
        not_found.headers["Content-Type"] = "application/json";
        return not_found;
    }

private:
    void add_route(HttpMethod method, const std::string& pattern, RouteHandler handler) {
        RouteEntry entry;
        entry.method = method;
        entry.pattern = pattern;
        entry.handler = std::move(handler);

        std::string regex_str;
        std::size_t pos = 0;
        while (pos < pattern.length()) {
            if (pattern[pos] == ':') {
                std::size_t end = pattern.find_first_of("/?", pos);
                if (end == std::string::npos) end = pattern.length();
                std::string param_name = pattern.substr(pos + 1, end - (pos + 1));
                entry.param_names.push_back(param_name);
                regex_str += "([^/]+)";
                pos = end;
            } else {
                if (pattern[pos] == '/' || pattern[pos] == '.' || pattern[pos] == '?') {
                    regex_str += '\\';
                }
                regex_str += pattern[pos];
                pos++;
            }
        }

        entry.regex_pattern = std::regex("^" + regex_str + "$");
        routes_.push_back(std::move(entry));
        std::cout << "🛣️ [GARUDA COWBOY] Registered route: " << pattern << std::endl;
    }

    std::vector<RouteEntry> routes_;
};

} // namespace garuda::beam
