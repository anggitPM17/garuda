#pragma once
// Garuda Standard Library: JSON Web Token (JWT) HS256 Engine (`jwt.hpp`)
#include "crypto.hpp"
#include "json.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam::stdlib {

class Base64Url {
public:
    static std::string encode(const std::string& input) {
        static const char b64_chars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        std::size_t in_len = input.length();
        const char* bytes = input.data();

        while (in_len--) {
            char_array_3[i++] = *(bytes++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i < 4); i++) ret += b64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j < 3; j++) char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            for (j = 0; (j < i + 1); j++) ret += b64_chars[char_array_4[j]];
        }

        // Convert Base64 -> Base64Url: replace '+' with '-', '/' with '_', remove '='
        std::string res;
        for (char c : ret) {
            if (c == '+') res += '-';
            else if (c == '/') res += '_';
            else if (c != '=') res += c;
        }
        return res;
    }

    static std::string decode(std::string input) {
        static const std::string b64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        // Restore Base64: replace '-' with '+', '_' with '/'
        for (char& c : input) {
            if (c == '-') c = '+';
            else if (c == '_') c = '/';
        }
        while (input.length() % 4 != 0) input += '=';

        auto is_base64 = [](unsigned char c) {
            return (isalnum(c) || (c == '+') || (c == '/'));
        };

        int in_len = static_cast<int>(input.length());
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;

        while (in_len-- && (input[in_] != '=') && is_base64(input[in_])) {
            char_array_4[i++] = input[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = static_cast<unsigned char>(b64_chars.find(char_array_4[i]));

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++) ret += char_array_3[i];
                i = 0;
            }
        }

        if (i) {
            for (j = 0; j < i; j++)
                char_array_4[j] = static_cast<unsigned char>(b64_chars.find(char_array_4[j]));

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

            for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
        }

        return ret;
    }
};

class Jwt {
public:
    static std::string encode(const std::map<std::string, Term>& payload_map, const std::string& secret) {
        // 1. Header
        std::string header_json = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
        std::string header_b64 = Base64Url::encode(header_json);

        // 2. Payload
        std::vector<Term> pairs;
        for (const auto& [k, v] : payload_map) {
            pairs.push_back(tuple_of({Term(k), v}));
        }
        std::string payload_json = Json::encode(list_of(pairs));
        std::string payload_b64 = Base64Url::encode(payload_json);

        // 3. Signature: Sha256::hash(header_b64 + "." + payload_b64 + "." + secret)
        std::string signing_input = header_b64 + "." + payload_b64;
        std::string raw_sig = Sha256::hash(signing_input + "." + secret);
        std::string sig_b64 = Base64Url::encode(raw_sig);

        return signing_input + "." + sig_b64;
    }

    static std::optional<std::map<std::string, Term>> decode(const std::string& token, const std::string& secret) {
        auto p1 = token.find('.');
        if (p1 == std::string::npos) return std::nullopt;
        auto p2 = token.find('.', p1 + 1);
        if (p2 == std::string::npos) return std::nullopt;

        std::string header_b64 = token.substr(0, p1);
        std::string payload_b64 = token.substr(p1 + 1, p2 - (p1 + 1));
        std::string sig_b64 = token.substr(p2 + 1);

        // 1. Verify Signature
        std::string signing_input = header_b64 + "." + payload_b64;
        std::string expected_raw_sig = Sha256::hash(signing_input + "." + secret);
        std::string expected_sig_b64 = Base64Url::encode(expected_raw_sig);

        if (sig_b64 != expected_sig_b64) {
            return std::nullopt; // Signature mismatch!
        }

        // 2. Decode & Parse Payload JSON
        std::string payload_json = Base64Url::decode(payload_b64);
        auto decoded_term_opt = Json::decode(payload_json);
        if (!decoded_term_opt.has_value()) return std::nullopt;

        std::map<std::string, Term> res;
        if (auto* lst = decoded_term_opt->get_if<garuda::beam::List>()) {
            const auto& items = seq_items(lst->items);
            for (const auto& item : items) {
                if (auto* tup = item.get_if<garuda::beam::Tuple>()) {
                    const auto& pair = seq_items(tup->items);
                    if (pair.size() == 2) {
                        std::string k = pair[0].get_if<std::string>() ? *pair[0].get_if<std::string>() : to_string(pair[0]);
                        res[k] = pair[1];
                    }
                }
            }
            return res;
        }

        return std::nullopt;
    }
};

} // namespace garuda::beam::stdlib
