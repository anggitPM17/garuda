#pragma once
// Garuda Standard Library: Native HTTP/1.1 Client (`http.hpp`)
// Mengimplementasikan HTTP GET & POST client tanpa dependensi libcurl/external library.
#include "../core/term.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#endif

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam::stdlib {

struct ParsedUrl {
    std::string protocol = "http";
    std::string host = "localhost";
    int port = 80;
    std::string path = "/";
};

struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> set_cookies;
    std::string body;
};

class Http {
public:
    static ParsedUrl parse_url(const std::string& url_str) {
        ParsedUrl result;
        std::string url = url_str;

        auto proto_pos = url.find("://");
        if (proto_pos != std::string::npos) {
            result.protocol = url.substr(0, proto_pos);
            url = url.substr(proto_pos + 3);
        }

        auto path_pos = url.find('/');
        if (path_pos != std::string::npos) {
            result.path = url.substr(path_pos);
            url = url.substr(0, path_pos);
        } else {
            result.path = "/";
        }

        auto port_pos = url.find(':');
        if (port_pos != std::string::npos) {
            result.host = url.substr(0, port_pos);
            result.port = std::stoi(url.substr(port_pos + 1));
        } else {
            result.host = url;
            result.port = (result.protocol == "https") ? 443 : 80;
        }

        return result;
    }

    static std::optional<HttpResponse> get(const std::string& url_str, int timeout_ms = 3000) {
        ParsedUrl url = parse_url(url_str);
        std::ostringstream req;
        req << "GET " << url.path << " HTTP/1.1\r\n";
        req << "Host: " << url.host << "\r\n";
        req << "User-Agent: Garuda-HTTP/1.0\r\n";
        req << "Connection: close\r\n\r\n";

        return send_http_request(url.host, url.port, req.str(), timeout_ms);
    }

    static std::optional<HttpResponse> post(const std::string& url_str, const std::string& body, const std::string& content_type = "application/json", int timeout_ms = 3000) {
        ParsedUrl url = parse_url(url_str);
        std::ostringstream req;
        req << "POST " << url.path << " HTTP/1.1\r\n";
        req << "Host: " << url.host << "\r\n";
        req << "User-Agent: Garuda-HTTP/1.0\r\n";
        req << "Content-Type: " << content_type << "\r\n";
        req << "Content-Length: " << body.length() << "\r\n";
        req << "Connection: close\r\n\r\n";
        req << body;

        return send_http_request(url.host, url.port, req.str(), timeout_ms);
    }

    static std::optional<HttpResponse> parse_response(const std::string& raw_resp) {
        auto header_end = raw_resp.find("\r\n\r\n");
        if (header_end == std::string::npos) return std::nullopt;

        std::string header_str = raw_resp.substr(0, header_end);
        std::string body_str = raw_resp.substr(header_end + 4);

        HttpResponse resp;
        resp.body = body_str;

        std::istringstream iss(header_str);
        std::string line;
        if (std::getline(iss, line)) {
            std::istringstream status_ss(line);
            std::string http_ver;
            status_ss >> http_ver >> resp.status_code;
            std::getline(status_ss, resp.status_text);
            if (!resp.status_text.empty() && resp.status_text.front() == ' ') {
                resp.status_text = resp.status_text.substr(1);
            }
        }

        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            auto colon = line.find(':');
            if (colon != std::string::npos) {
                std::string k = line.substr(0, colon);
                std::string v = line.substr(colon + 1);
                if (!v.empty() && v.front() == ' ') v = v.substr(1);
                std::transform(k.begin(), k.end(), k.begin(), ::tolower);
                resp.headers[k] = v;
            }
        }

        return resp;
    }

private:
    static std::optional<HttpResponse> send_http_request(const std::string& host, int port, const std::string& request_data, int timeout_ms) {
#ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

#ifdef _WIN32
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) return std::nullopt;
        DWORD tv = timeout_ms;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

        int nodelay = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));
#else
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) return std::nullopt;
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

        int nodelay = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));
#endif

        if (host == "127.0.0.1" || host == "localhost") {
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
#ifdef _WIN32
                closesocket(sock);
#else
                close(sock);
#endif
                return std::nullopt;
            }
        } else {
            struct addrinfo hints{}, *res = nullptr;
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            std::string port_str = std::to_string(port);
            if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0 || !res) {
#ifdef _WIN32
                closesocket(sock);
#else
                close(sock);
#endif
                return std::nullopt;
            }
            if (connect(sock, res->ai_addr, static_cast<int>(res->ai_addrlen)) != 0) {
                freeaddrinfo(res);
#ifdef _WIN32
                closesocket(sock);
#else
                close(sock);
#endif
                return std::nullopt;
            }
            freeaddrinfo(res);
        }

        send(sock, request_data.c_str(), static_cast<int>(request_data.length()), 0);

        std::string response_data;
        char buf[2048];
        int bytes_read = 0;
        while ((bytes_read = recv(sock, buf, sizeof(buf) - 1, 0)) > 0) {
            buf[bytes_read] = '\0';
            response_data += buf;
        }

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif

        return parse_response(response_data);
    }
};

} // namespace garuda::beam::stdlib
