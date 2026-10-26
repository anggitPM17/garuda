#pragma once
// Garuda Language Web Framework: Web Framework Core (`web.hpp`)
// Mengimplementasikan Router, Dynamic Controllers, Response Helpers, View Templating, & Database Integration.
#include "http_server.hpp"
#include "json.hpp"
#include "db.hpp"
#include "view.hpp"

#include <functional>
#include <memory>
#include <string>

namespace garuda::beam::stdlib {

class WebFramework {
public:
    WebFramework() = default;

    static HttpResponse json_response(const std::string& json_str, int status_code = 200) {
        HttpResponse res = HttpServer::make_json(json_str);
        res.status_code = status_code;
        return res;
    }

    static HttpResponse html_response(const std::string& html_str, int status_code = 200) {
        HttpResponse res = HttpServer::make_ok(html_str, "text/html");
        res.status_code = status_code;
        return res;
    }

    static HttpResponse render(const std::string& template_str, const Term& data) {
        std::string rendered = ViewEngine::render_template(template_str, data);
        return html_response(rendered);
    }

    static HttpResponse render_with_layout(const std::string& layout_str, const std::string& template_str, const Term& data) {
        std::string rendered = ViewEngine::render_with_layout(layout_str, template_str, data);
        return html_response(rendered);
    }

    static HttpResponse redirect_response(const std::string& location_url) {
        HttpResponse res;
        res.status_code = 302;
        res.status_text = "Found";
        res.headers["Location"] = location_url;
        res.body = "";
        return res;
    }

    static HttpResponse not_found_response(const std::string& message = "404 Not Found") {
        return HttpServer::make_not_found(message);
    }

    void get(const std::string& path, HttpHandler handler) {
        router_.get(path, std::move(handler));
    }

    void post(const std::string& path, HttpHandler handler) {
        router_.post(path, std::move(handler));
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

    Router& router() { return router_; }

private:
    Router router_;
    HttpServer server_;
};

} // namespace garuda::beam::stdlib
