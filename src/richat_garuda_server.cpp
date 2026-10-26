#include "../include/garuda/beam/beam.hpp"
#include "../include/garuda/beam/net/websocket_handler.hpp"
#include "../include/garuda/beam/crypto/x3dh.hpp"
#include <iostream>

using namespace garuda::beam;
using namespace garuda::elixir;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA GVM SERVER FOR RICHAT v67.0.0-production\n";
    std::cout << "=======================================================\n\n";

    // 1. Initialize Runtime & Work-Stealing Multi-Core Scheduler
    Runtime rt;
    EventLoop loop(rt, ClockMode::Real);

    // 2. Initialize Mnesia Database with RICHAT Schema
    Mnesia db("./garuda_richat_db");
    db.init_richat_schema();

    // 3. Initialize Media Storage & X3DH Crypto Engine
    MediaStorageEngine media_engine("./garuda_richat_media");
    X3DhEngine x3dh_engine;
    RichatWsHandler ws_handler(rt);

    // 4. Initialize Cowboy-Grade Router for RICHAT REST API
    CowboyRouter router;

    // Route: Auth Login
    router.post("/api/auth/login", [&](const HttpRequest&, const std::map<std::string, std::string>&) {
        HttpResponse resp;
        std::string secret = "richat_v2_production_secret_key";
        std::string token = Crypto::generate_jwt("C4D1IDDF32", secret);
        resp.status_code = 200;
        resp.body = "{\"status\":\"ok\",\"token\":\"" + token + "\",\"user_id\":\"C4D1IDDF32\",\"display_name\":\"Emulator Test\"}";
        resp.headers["Content-Type"] = "application/json";
        return resp;
    });

    // Route: Contacts GET
    router.get("/api/contacts", [](const HttpRequest&, const std::map<std::string, std::string>&) {
        HttpResponse resp;
        resp.status_code = 200;
        resp.body = "{\"contacts\":[],\"status\":\"ok\"}";
        resp.headers["Content-Type"] = "application/json";
        return resp;
    });

    // Route: Moments POST
    router.post("/api/moment", [&](const HttpRequest& req, const std::map<std::string, std::string>&) {
        HttpResponse resp;
        std::string moment_id = Crypto::generate_uuid("moment_");
        db.write("richat_moment_post", Term(moment_id), Term(req.body));
        resp.status_code = 200;
        resp.body = "{\"id\":\"" + moment_id + "\",\"status\":\"ok\"}";
        resp.headers["Content-Type"] = "application/json";
        return resp;
    });

    // Route: Group Metadata PUT
    router.put("/api/groups/:id/metadata", [&](const HttpRequest& req, const std::map<std::string, std::string>& bindings) {
        HttpResponse resp;
        std::string group_id = bindings.at("id");
        db.write("richat_group", Term(group_id), Term(req.body));
        resp.status_code = 200;
        resp.body = "{\"group_id\":\"" + group_id + "\",\"status\":\"success\"}";
        resp.headers["Content-Type"] = "application/json";
        return resp;
    });

    // Route: Telemetry Metrics
    router.get("/metrics", [&](const HttpRequest&, const std::map<std::string, std::string>&) {
        HttpResponse resp;
        resp.status_code = 200;
        resp.body = TelemetryEngine::export_prometheus(rt.stats(), ws_handler.online_count());
        resp.headers["Content-Type"] = "text/plain";
        return resp;
    });

    // 5. Initialize HttpWsServer on Port 8080
    HttpWsServer server(loop, 8080);
    server.set_http_handler([&](const HttpRequest& req) {
        return router.dispatch(req);
    });

    if (server.start()) {
        std::cout << "✨ [GARUDA RICHAT SERVER] Server online, Mnesia DB active, listening on port 8080!\n";
        loop.run();
    }

    return 0;
}
