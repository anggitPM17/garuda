#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <sstream>
#include <cstdio>

namespace garuda::nusantara {

// Entitas yang terhubung dalam ekosistem Garuda Nusantara
enum class EntityTarget {
    RICHAT_APP,
    GARUDA_GVM,
    STADION_UI,
    ANDROID_STUDIO,
    AUTOPILOT_AI
};

// Aksi yang bisa dilakukan antar entitas
enum class NusantaraAction {
    UPDATE_CODE,      // Perintah update kode / hot-reload
    RELOAD_ASSETS,    // Perintah memuat ulang aset UI
    RUN_TEST,         // Perintah menjalankan pengujian
    GET_METRICS,      // Permintaan data telemetri / kesehatan
    PUSH_PATCH,       // Mengirim patch kode hasil perbaikan AI
    BROADCAST_MSG     // Pesan teks biasa
};

inline const char* to_string(EntityTarget target) {
    switch (target) {
        case EntityTarget::RICHAT_APP: return "RICHAT_APP";
        case EntityTarget::GARUDA_GVM: return "GARUDA_GVM";
        case EntityTarget::STADION_UI: return "STADION_UI";
        case EntityTarget::ANDROID_STUDIO: return "ANDROID_STUDIO";
        case EntityTarget::AUTOPILOT_AI: return "AUTOPILOT_AI";
    }
    return "UNKNOWN";
}

inline bool from_string(const std::string& str, EntityTarget& out) {
    if (str == "RICHAT_APP") { out = EntityTarget::RICHAT_APP; return true; }
    if (str == "GARUDA_GVM") { out = EntityTarget::GARUDA_GVM; return true; }
    if (str == "STADION_UI") { out = EntityTarget::STADION_UI; return true; }
    if (str == "ANDROID_STUDIO") { out = EntityTarget::ANDROID_STUDIO; return true; }
    if (str == "AUTOPILOT_AI") { out = EntityTarget::AUTOPILOT_AI; return true; }
    return false;
}

inline const char* to_string(NusantaraAction action) {
    switch (action) {
        case NusantaraAction::UPDATE_CODE: return "UPDATE_CODE";
        case NusantaraAction::RELOAD_ASSETS: return "RELOAD_ASSETS";
        case NusantaraAction::RUN_TEST: return "RUN_TEST";
        case NusantaraAction::GET_METRICS: return "GET_METRICS";
        case NusantaraAction::PUSH_PATCH: return "PUSH_PATCH";
        case NusantaraAction::BROADCAST_MSG: return "BROADCAST_MSG";
    }
    return "UNKNOWN";
}

inline bool from_string(const std::string& str, NusantaraAction& out) {
    if (str == "UPDATE_CODE") { out = NusantaraAction::UPDATE_CODE; return true; }
    if (str == "RELOAD_ASSETS") { out = NusantaraAction::RELOAD_ASSETS; return true; }
    if (str == "RUN_TEST") { out = NusantaraAction::RUN_TEST; return true; }
    if (str == "GET_METRICS") { out = NusantaraAction::GET_METRICS; return true; }
    if (str == "PUSH_PATCH") { out = NusantaraAction::PUSH_PATCH; return true; }
    if (str == "BROADCAST_MSG") { out = NusantaraAction::BROADCAST_MSG; return true; }
    return false;
}

inline std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<int>(c));
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
                break;
        }
    }
    return out;
}

namespace detail {

struct JsonParser {
    const std::string& src;
    std::size_t pos = 0;

    void skip_whitespace() {
        while (pos < src.size() && (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r')) {
            pos++;
        }
    }

    bool parse_string(std::string& out) {
        skip_whitespace();
        if (pos >= src.size() || src[pos] != '"') return false;
        pos++; // skip opening quote
        out.clear();
        while (pos < src.size()) {
            char c = src[pos++];
            if (c == '"') {
                return true;
            }
            if (c == '\\') {
                if (pos >= src.size()) return false;
                char esc = src[pos++];
                switch (esc) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    case 'u': {
                        if (pos + 4 > src.size()) return false;
                        std::string hex_str = src.substr(pos, 4);
                        pos += 4;
                        try {
                            unsigned long val = std::stoul(hex_str, nullptr, 16);
                            if (val <= 0x7F) {
                                out += static_cast<char>(val);
                            } else {
                                out += '?';
                            }
                        } catch (...) {
                            return false;
                        }
                        break;
                    }
                    default: out += esc; break;
                }
            } else {
                out += c;
            }
        }
        return false;
    }

    bool consume(char expected) {
        skip_whitespace();
        if (pos < src.size() && src[pos] == expected) {
            pos++;
            return true;
        }
        return false;
    }
};

} // namespace detail

struct NusantaraMessage {
    std::string message_id;
    EntityTarget sender;
    EntityTarget receiver;
    NusantaraAction action;
    std::map<std::string, std::string> payload;

    std::string to_json() const {
        std::string json = "{";
        json += "\"message_id\":\"" + json_escape(message_id) + "\",";
        json += "\"sender\":\"" + json_escape(to_string(sender)) + "\",";
        json += "\"receiver\":\"" + json_escape(to_string(receiver)) + "\",";
        json += "\"action\":\"" + json_escape(to_string(action)) + "\",";
        json += "\"payload\":{";
        bool first = true;
        for (const auto& [k, v] : payload) {
            if (!first) json += ",";
            first = false;
            json += "\"" + json_escape(k) + "\":\"" + json_escape(v) + "\"";
        }
        json += "}}";
        return json;
    }

    static std::optional<NusantaraMessage> from_json(const std::string& json) {
        if (json.empty()) return std::nullopt;

        detail::JsonParser parser{json, 0};

        if (!parser.consume('{')) return std::nullopt;

        NusantaraMessage msg;
        bool has_id = false, has_sender = false, has_receiver = false, has_action = false;

        while (true) {
            parser.skip_whitespace();
            if (parser.consume('}')) break;

            std::string key;
            if (!parser.parse_string(key)) return std::nullopt;
            if (!parser.consume(':')) return std::nullopt;

            if (key == "message_id") {
                if (!parser.parse_string(msg.message_id)) return std::nullopt;
                has_id = true;
            } else if (key == "sender") {
                std::string s;
                if (!parser.parse_string(s) || !from_string(s, msg.sender)) return std::nullopt;
                has_sender = true;
            } else if (key == "receiver") {
                std::string r;
                if (!parser.parse_string(r) || !from_string(r, msg.receiver)) return std::nullopt;
                has_receiver = true;
            } else if (key == "action") {
                std::string a;
                if (!parser.parse_string(a) || !from_string(a, msg.action)) return std::nullopt;
                has_action = true;
            } else if (key == "payload") {
                if (!parser.consume('{')) return std::nullopt;
                while (true) {
                    parser.skip_whitespace();
                    if (parser.consume('}')) break;

                    std::string pkey, pval;
                    if (!parser.parse_string(pkey)) return std::nullopt;
                    if (!parser.consume(':')) return std::nullopt;
                    if (!parser.parse_string(pval)) return std::nullopt;

                    msg.payload[pkey] = pval;

                    parser.skip_whitespace();
                    parser.consume(',');
                }
            } else {
                std::string dummy;
                if (!parser.parse_string(dummy)) {
                    while (parser.pos < parser.src.size() && parser.src[parser.pos] != ',' && parser.src[parser.pos] != '}') {
                        parser.pos++;
                    }
                }
            }

            parser.skip_whitespace();
            parser.consume(',');
        }

        if (!has_id || !has_sender || !has_receiver || !has_action) {
            return std::nullopt;
        }

        return msg;
    }
};

} // namespace garuda::nusantara
