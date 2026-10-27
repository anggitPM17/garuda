#include "../include/garuda/beam/io/http_ws_server.hpp"
#include "../include/garuda/beam/stdlib/query_builder.hpp"
#include "../include/garuda/beam/stdlib/json.hpp"
#include "../include/garuda/beam/core/runtime.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

using namespace garuda::beam;
using namespace garuda::beam::stdlib;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 10) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1); \
    }
}

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

// Minimal WebSocket Client Frame Encoder
std::vector<std::uint8_t> encode_ws_client_text_frame(const std::string& payload) {
    std::vector<std::uint8_t> frame;
    frame.push_back(0x81); // FIN (1) + Opcode (1 = Text)

    std::size_t len = payload.length();
    if (len <= 125) {
        frame.push_back(static_cast<std::uint8_t>(0x80 | len)); // Mask bit = 1
    } else {
        frame.push_back(0x80 | 126);
        frame.push_back(static_cast<std::uint8_t>((len >> 8) & 0xFF));
        frame.push_back(static_cast<std::uint8_t>(len & 0xFF));
    }

    // Masking Key
    std::uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
    for (int i = 0; i < 4; ++i) frame.push_back(mask[i]);

    // Masked Payload
    for (std::size_t i = 0; i < len; ++i) {
        frame.push_back(static_cast<std::uint8_t>(payload[i]) ^ mask[i % 4]);
    }

    return frame;
}

struct WsDecodedFrame {
    std::uint8_t opcode = 0;
    std::string payload;
};

WsDecodedFrame decode_ws_frame(const std::uint8_t* buf, std::size_t len) {
    WsDecodedFrame frame;
    if (len < 2) return frame;

    frame.opcode = buf[0] & 0x0F;
    bool masked = (buf[1] & 0x80) != 0;
    std::size_t payload_len = buf[1] & 0x7F;
    std::size_t offset = 2;

    if (payload_len == 126) {
        if (len < 4) return frame;
        payload_len = (buf[2] << 8) | buf[3];
        offset = 4;
    }

    std::uint8_t mask[4] = {0, 0, 0, 0};
    if (masked) {
        if (len < offset + 4) return frame;
        for (int i = 0; i < 4; ++i) mask[i] = buf[offset++];
    }

    std::string text;
    text.reserve(payload_len);
    for (std::size_t i = 0; i < payload_len && (offset + i) < len; ++i) {
        std::uint8_t b = buf[offset + i];
        if (masked) b ^= mask[i % 4];
        text += static_cast<char>(b);
    }

    frame.payload = text;
    return frame;
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA RICHAT BACKEND DEMO E2E SUITE\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Full RICHAT WebSocket Message Ingestion & HTTP Retrieval ===
    run_test_with_timeout("Test 1: RICHAT WebSocket Ingestion & HTTP Retrieval", []() {
        sqlite3* db = nullptr;
        int rc = sqlite3_open(":memory:", &db);
        GARUDA_CHECK(rc == SQLITE_OK && db != nullptr, "sqlite3_open(:memory:) must succeed");

        const char* init_sql =
            "CREATE TABLE messages ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "sender TEXT NOT NULL, "
            "content TEXT NOT NULL, "
            "timestamp INTEGER NOT NULL"
            ");";
        char* err_msg = nullptr;
        rc = sqlite3_exec(db, init_sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            if (err_msg) sqlite3_free(err_msg);
            GARUDA_CHECK(false, "CREATE TABLE messages failed");
        }

        Runtime rt;
        EventLoop loop(rt);
        int port = 8110;

        HttpWsServer server(loop, port);

        // Server WebSocket Handler
        server.set_ws_handler([db](const std::string& msg_str, std::function<void(const std::string&)> send_reply) {
            std::string sender = extract_json_field(msg_str, "sender");
            std::string content = extract_json_field(msg_str, "content");
            if (sender.empty()) sender = "Anonymous";
            if (content.empty()) content = msg_str;

            auto now_ts = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

            // Insert message to DB via QueryBuilder
            QueryBuilder qb;
            qb.insert_into("messages", {
                {"sender", sender},
                {"content", content},
                {"timestamp", std::to_string(now_ts)}
            });
            QueryBuilder::execute_prepared(db, qb.to_sql());

            std::ostringstream resp;
            resp << "{\"status\":\"ok\",\"sender\":\"" << sender << "\",\"content\":\"" << content << "\"}";
            send_reply(resp.str());
        });

        // Server HTTP Handler
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
        GARUDA_CHECK(started, "HttpWsServer must start on port 8110");

        std::thread loop_thread([&loop]() {
            loop.run();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // 1. WebSocket Client Send Message
        socket_t ws_client = ::socket(AF_INET, SOCK_STREAM, 0);
        GARUDA_CHECK(ws_client != INVALID_SOCK, "WS Client socket creation must succeed");

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<u_short>(port));
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        int conn_rc = ::connect(ws_client, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        GARUDA_CHECK(conn_rc == 0, "WS Connect to 127.0.0.1:8110 must succeed");

        std::string hs_req =
            "GET /ws HTTP/1.1\r\n"
            "Host: 127.0.0.1:8110\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";

        ::send(ws_client, hs_req.c_str(), static_cast<int>(hs_req.length()), 0);

        char buf[1024];
        int bytes = ::recv(ws_client, buf, sizeof(buf) - 1, 0);
        GARUDA_CHECK(bytes > 0 && std::string(buf, bytes).find("101 Switching Protocols") != std::string::npos, "WS Handshake must return 101");

        // Send JSON Message via WS Frame
        std::string json_msg = "{\"sender\":\"Budi\",\"content\":\"Halo\"}";
        auto frame_data = encode_ws_client_text_frame(json_msg);
        ::send(ws_client, reinterpret_cast<const char*>(frame_data.data()), static_cast<int>(frame_data.size()), 0);

        std::uint8_t recv_buf[1024];
        int r_bytes = ::recv(ws_client, reinterpret_cast<char*>(recv_buf), sizeof(recv_buf), 0);
        GARUDA_CHECK(r_bytes > 0, "Must receive WS reply frame");

        auto server_reply = decode_ws_frame(recv_buf, static_cast<std::size_t>(r_bytes));
        GARUDA_CHECK(server_reply.opcode == 0x1, "WS Opcode must be 0x1");
        GARUDA_CHECK(server_reply.payload.find("\"status\":\"ok\"") != std::string::npos, "WS Reply status must be 'ok'");
        GARUDA_CHECK(server_reply.payload.find("\"sender\":\"Budi\"") != std::string::npos, "WS Reply sender must be 'Budi'");
        GARUDA_CHECK(server_reply.payload.find("\"content\":\"Halo\"") != std::string::npos, "WS Reply content must be 'Halo'");

        CLOSE_SOCK(ws_client);

        // 2. Verify Database Persistence Direct Query
        QueryBuilder sel;
        sel.select({"sender", "content"}).from("messages");
        auto db_rows = QueryBuilder::fetch_all_prepared(db, sel.to_sql());
        GARUDA_CHECK(db_rows.size() == 1, "Database must contain exactly 1 saved message");
        GARUDA_CHECK(db_rows[0][0] == "Budi" && db_rows[0][1] == "Halo", "Database row values must match 'Budi' and 'Halo'");

        // 3. HTTP GET /messages Retrieval Test
        socket_t http_client = ::socket(AF_INET, SOCK_STREAM, 0);
        GARUDA_CHECK(http_client != INVALID_SOCK, "HTTP Client socket creation must succeed");

        conn_rc = ::connect(http_client, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        GARUDA_CHECK(conn_rc == 0, "HTTP Connect to 127.0.0.1:8110 must succeed");

        std::string http_req = "GET /messages HTTP/1.1\r\nHost: 127.0.0.1:8110\r\nConnection: close\r\n\r\n";
        ::send(http_client, http_req.c_str(), static_cast<int>(http_req.length()), 0);

        char http_buf[2048];
        int h_bytes = ::recv(http_client, http_buf, sizeof(http_buf) - 1, 0);
        GARUDA_CHECK(h_bytes > 0, "Must receive HTTP response from /messages");
        http_buf[h_bytes] = '\0';

        std::string http_resp(http_buf, h_bytes);
        GARUDA_CHECK(http_resp.find("200 OK") != std::string::npos, "HTTP /messages must return 200 OK");
        GARUDA_CHECK(http_resp.find("\"sender\":\"Budi\"") != std::string::npos, "HTTP /messages JSON must contain sender 'Budi'");
        GARUDA_CHECK(http_resp.find("\"content\":\"Halo\"") != std::string::npos, "HTTP /messages JSON must contain content 'Halo'");

        CLOSE_SOCK(http_client);

        server.stop();
        loop.stop();
        if (loop_thread.joinable()) loop_thread.join();
        sqlite3_close(db);

        std::cout << "  ✓ Test 1: RICHAT WebSocket Ingestion & HTTP Retrieval PASS\n";
    }, 10);

    std::cout << "\n🎉 ALL RICHAT BACKEND DEMO TESTS PASSED PERFECTLY!\n";
    return 0;
}
