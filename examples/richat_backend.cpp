#include "../include/garuda/beam/io/http_ws_server.hpp"
#include "../include/garuda/beam/stdlib/query_builder.hpp"
#include "../include/garuda/beam/stdlib/json.hpp"
#include "../include/garuda/beam/core/runtime.hpp"
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

using namespace garuda::beam;
using namespace garuda::beam::stdlib;

static std::string extract_json_field(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    auto colon = json.find(':', pos);
    if (colon == std::string::npos) return "";
    auto start_quote = json.find('"', colon);
    if (start_quote == std::string::npos) return "";
    auto end_quote = json.find('"', start_quote + 1);
    if (end_quote == std::string::npos) return "";
    return json.substr(start_quote + 1, end_quote - start_quote - 1);
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "🚀 GARUDA RICHAT BACKEND DEMO SERVER (PORT 8080)\n";
    std::cout << "=======================================================\n\n";

    // 1. Initialize SQLite Database
    sqlite3* db = nullptr;
    int rc = sqlite3_open("richat.db", &db);
    if (rc != SQLITE_OK || !db) {
        std::cerr << "Failed to open richat.db" << std::endl;
        return 1;
    }

    const char* init_sql =
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "sender TEXT NOT NULL, "
        "content TEXT NOT NULL, "
        "timestamp INTEGER NOT NULL"
        ");";
    char* err_msg = nullptr;
    sqlite3_exec(db, init_sql, nullptr, nullptr, &err_msg);

    Runtime rt;
    EventLoop loop(rt);
    int port = 8080;

    HttpWsServer server(loop, port);

    // 2. Register WebSocket Handler
    server.set_ws_handler([db](const std::string& msg_str, std::function<void(const std::string&)> send_reply) {
        std::string sender = extract_json_field(msg_str, "sender");
        std::string content = extract_json_field(msg_str, "content");
        if (sender.empty()) sender = "Anonymous";
        if (content.empty()) content = msg_str;

        auto now_ts = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        // Save to SQLite via QueryBuilder
        QueryBuilder qb;
        qb.insert_into("messages", {
            {"sender", sender},
            {"content", content},
            {"timestamp", std::to_string(now_ts)}
        });
        QueryBuilder::execute_prepared(db, qb.to_sql());

        // Send Reply
        std::ostringstream resp;
        resp << "{\"status\":\"ok\",\"sender\":\"" << sender << "\",\"content\":\"" << content << "\"}";
        send_reply(resp.str());
    });

    // 3. Register HTTP GET /messages Handler
    server.set_http_handler([db](const HttpRequest& req) {
        HttpResponse resp;
        if (req.path == "/messages") {
            QueryBuilder sel;
            sel.select({"id", "sender", "content", "timestamp"}).from("messages");
            auto rows = QueryBuilder::fetch_all_prepared(db, sel.to_sql());

            std::ostringstream json_out;
            json_out << "[";
            for (std::size_t i = 0; i < rows.size(); ++i) {
                if (i > 0) json_out << ",";
                json_out << "{\"id\":" << rows[i][0]
                         << ",\"sender\":\"" << rows[i][1] << "\""
                         << ",\"content\":\"" << rows[i][2] << "\""
                         << ",\"timestamp\":" << rows[i][3] << "}";
            }
            json_out << "]";

            resp.status_code = 200;
            resp.body = json_out.str();
            resp.headers["Content-Type"] = "application/json";
        } else {
            resp.status_code = 404;
            resp.body = "{\"error\":\"Not Found\"}";
        }
        return resp;
    });

    bool started = server.start();
    if (started) {
        std::cout << "⚡ RICHAT Backend listening on http/ws://0.0.0.0:8080" << std::endl;
        loop.run();
    }

    sqlite3_close(db);
    return 0;
}
