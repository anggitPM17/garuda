#pragma once
// Garuda GVM: Term <-> JSON Serializer (`term_json.hpp`)
#include "../core/term.hpp"
#include "../stdlib/json.hpp"

#include <optional>
#include <string>

namespace garuda::beam {

class TermJson {
public:
    static std::string serialize(const Term& term) {
        return stdlib::Json::encode(term);
    }

    static std::optional<Term> deserialize(const std::string& json_str) {
        return stdlib::Json::decode(json_str);
    }
};

} // namespace garuda::beam
