#pragma once
// Garuda Standard Library: Full-Stack Integration Bridge (`fullstack_bridge.hpp`)
// Menghubungkan Router HttpServer, Database Persistence, dan JSON Parser dalam satu stack terintegrasi.
#include "http_server.hpp"
#include "json.hpp"
#include "db.hpp"

#include <memory>
#include <string>

namespace garuda::beam::stdlib {

class FullStackApp {
public:
    explicit FullStackApp(std::string db_path) : db_(std::move(db_path)) {
        setup_routes();
    }

    bool start(int port) {
        return server_.start(port, router_);
    }

    void stop() {
        server_.stop();
    }

    bool is_running() const {
        return server_.is_running();
    }

    Database& db() { return db_; }

private:
    void setup_routes() {
        // 1. GET / -> Healthcheck
        router_.get("/", [](const HttpRequest&) {
            return HttpServer::make_json("{\"app\":\"Garuda Fullstack\",\"status\":\"ok\"}");
        });

        // 2. POST /users -> Create User in Database
        router_.post("/users", [this](const HttpRequest& req) {
            auto dec = Json::decode(req.body);
            if (!dec.has_value()) {
                HttpResponse res;
                res.status_code = 400;
                res.status_text = "Bad Request";
                res.body = "{\"error\":\"invalid_json\"}";
                res.headers["content-type"] = "application/json";
                return res;
            }

            std::string user_id = "usr_101";
            db_.insert("users", user_id, *dec);
            db_.flush();

            HttpResponse res;
            res.status_code = 201;
            res.status_text = "Created";
            res.body = "{\"id\":\"" + user_id + "\",\"status\":\"created\"}";
            res.headers["content-type"] = "application/json";
            return res;
        });

        // 3. GET /users/usr_101 -> Retrieve User by ID
        router_.get("/users/usr_101", [this](const HttpRequest&) {
            auto user_rec = db_.get("users", "usr_101");
            if (!user_rec.has_value()) {
                return HttpServer::make_not_found("{\"error\":\"not_found\"}");
            }

            std::string user_json = Json::encode(*user_rec);
            return HttpServer::make_json(user_json);
        });
    }

    Database db_;
    Router router_;
    HttpServer server_;
};

} // namespace garuda::beam::stdlib
