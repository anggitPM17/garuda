#pragma once
// Garuda Language Web Framework: Real-Time LiveView Engine (`live_view.hpp`)
// Mengimplementasikan Server-Rendered Real-Time Components, WebSocket State Management, & DOM Diff/Patch.
#include "../core/term.hpp"
#include "view.hpp"
#include "json.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam::stdlib {

struct LiveViewSession {
    std::string session_id;
    std::string template_str;
    Term state;
    std::string last_rendered_html;
};

using EventHandler = std::function<Term(const std::string& event, const Term& payload, const Term& current_state)>;

class LiveViewEngine {
public:
    LiveViewEngine() = default;

    std::string mount(const std::string& session_id, const std::string& template_str, const Term& initial_state) {
        std::lock_guard<std::mutex> lock(mu_);
        LiveViewSession sess;
        sess.session_id = session_id;
        sess.template_str = template_str;
        sess.state = initial_state;
        sess.last_rendered_html = ViewEngine::render_template(template_str, initial_state);

        sessions_[session_id] = sess;
        return sess.last_rendered_html;
    }

    std::optional<std::string> push_event(
        const std::string& session_id,
        const std::string& event,
        const Term& payload,
        EventHandler handler
    ) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = sessions_.find(session_id);
        if (it == sessions_.end()) return std::nullopt;

        // 1. Process event and update state
        Term new_state = handler(event, payload, it->second.state);
        it->second.state = new_state;

        // 2. Re-render HTML with new state
        std::string new_html = ViewEngine::render_template(it->second.template_str, new_state);

        // 3. Compute DOM Diff Patch
        std::string diff_patch = compute_dom_diff(it->second.last_rendered_html, new_html);
        it->second.last_rendered_html = new_html;

        // 4. Return JSON Diff Patch payload
        Term diff_res = list_of({
            tuple_of({Term("event"), Term(event)}),
            tuple_of({Term("patch"), Term(diff_patch)}),
            tuple_of({Term("html"), Term(new_html)})
        });

        return Json::encode(diff_res);
    }

    std::optional<Term> get_state(const std::string& session_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = sessions_.find(session_id);
        if (it != sessions_.end()) return it->second.state;
        return std::nullopt;
    }

    std::optional<std::string> get_rendered_html(const std::string& session_id) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = sessions_.find(session_id);
        if (it != sessions_.end()) return it->second.last_rendered_html;
        return std::nullopt;
    }

    void unmount(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(mu_);
        sessions_.erase(session_id);
    }

    static std::string compute_dom_diff(const std::string& old_html, const std::string& new_html) {
        if (old_html == new_html) return "{}";

        std::ostringstream patch;
        patch << "{\"diff\":\"updated\",\"from\":\"" << old_html << "\",\"to\":\"" << new_html << "\"}";
        return patch.str();
    }

private:
    mutable std::mutex mu_;
    std::unordered_map<std::string, LiveViewSession> sessions_;
};

} // namespace garuda::beam::stdlib
