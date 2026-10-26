#pragma once
// Garuda Standard Library: Native Result Type Engine (`result.hpp`)
// Mengimplementasikan Result::ok(Val), Result::error(Reason), is_ok, is_error, unwrap, unwrap_or, & map.
#include "../core/term.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace garuda::beam::stdlib {

class Result {
public:
    static Term ok(const Term& val) {
        return tuple_of({Term(Atom{"ok"}), val});
    }

    static Term error(const Term& reason) {
        return tuple_of({Term(Atom{"error"}), reason});
    }

    static bool is_ok(const Term& res) {
        if (auto* tup = res.get_if<Tuple>()) {
            const auto& items = seq_items(tup->items);
            if (items.size() == 2 && items[0].get_if<Atom>() && items[0].get_if<Atom>()->name == "ok") {
                return true;
            }
        }
        return false;
    }

    static bool is_error(const Term& res) {
        if (auto* tup = res.get_if<Tuple>()) {
            const auto& items = seq_items(tup->items);
            if (items.size() == 2 && items[0].get_if<Atom>() && items[0].get_if<Atom>()->name == "error") {
                return true;
            }
        }
        return false;
    }

    static Term unwrap(const Term& res) {
        if (is_ok(res)) {
            auto* tup = res.get_if<Tuple>();
            return seq_items(tup->items)[1];
        }
        throw std::runtime_error("Result::unwrap failed on error: " + to_string(res));
    }

    static Term unwrap_or(const Term& res, const Term& default_val) {
        if (is_ok(res)) {
            auto* tup = res.get_if<Tuple>();
            return seq_items(tup->items)[1];
        }
        return default_val;
    }

    static Term map(const Term& res, std::function<Term(const Term&)> fn) {
        if (is_ok(res)) {
            auto* tup = res.get_if<Tuple>();
            return ok(fn(seq_items(tup->items)[1]));
        }
        return res;
    }
};

} // namespace garuda::beam::stdlib
