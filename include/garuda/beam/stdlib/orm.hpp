#pragma once
// Garuda Language Web Framework: Native ORM & Schema Migration Engine (`orm.hpp`)
// Mengimplementasikan Model Registry, Query Builder, Relationships (has_many/belongs_to), & Schema Migrations.
#include "../core/term.hpp"
#include "db.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam::stdlib {

struct FieldDef {
    std::string name;
    std::string type; // "string", "integer", "boolean"
};

struct ModelDef {
    std::string name;
    std::map<std::string, std::string> fields;
    std::string has_many_target;
    std::string belongs_to_target;
    std::string foreign_key;
};

class OrmEngine {
public:
    explicit OrmEngine(Database& db) : db_(db) {}

    void define_model(
        const std::string& name,
        const std::map<std::string, std::string>& fields,
        const std::string& has_many = "",
        const std::string& belongs_to = "",
        const std::string& fk = ""
    ) {
        ModelDef def;
        def.name = name;
        def.fields = fields;
        def.has_many_target = has_many;
        def.belongs_to_target = belongs_to;
        def.foreign_key = fk;
        models_[name] = def;
    }

    bool insert(const std::string& model_name, const std::string& id, const Term& record) {
        auto it = models_.find(model_name);
        if (it == models_.end()) return false;
        return db_.insert(model_name, id, record);
    }

    std::optional<Term> find(const std::string& model_name, const std::string& id) const {
        return db_.get(model_name, id);
    }

    bool update(const std::string& model_name, const std::string& id, const Term& new_record) {
        return db_.update(model_name, id, new_record);
    }

    bool remove(const std::string& model_name, const std::string& id) {
        return db_.remove(model_name, id);
    }

    std::vector<Term> where(const std::string& model_name, const std::string& field, const std::string& value) const {
        std::vector<Term> all_records = db_.query_all(model_name);
        std::vector<Term> filtered;

        for (const auto& rec : all_records) {
            if (auto* list_val = rec.get_if<garuda::beam::List>()) {
                for (const auto& item : seq_items(list_val->items)) {
                    if (auto* tup = item.get_if<garuda::beam::Tuple>()) {
                        const auto& pair = seq_items(tup->items);
                        if (pair.size() == 2) {
                            std::string k = pair[0].get_if<std::string>() ? *pair[0].get_if<std::string>() : to_string(pair[0]);
                            std::string v = pair[1].get_if<std::string>() ? *pair[1].get_if<std::string>() : to_string(pair[1]);
                            if (k == field && v == value) {
                                filtered.push_back(rec);
                                break;
                            }
                        }
                    }
                }
            }
        }
        return filtered;
    }

    std::vector<Term> where_limit(const std::string& model_name, const std::string& field, const std::string& value, std::size_t limit_n) const {
        auto matches = where(model_name, field, value);
        if (matches.size() > limit_n) {
            matches.resize(limit_n);
        }
        return matches;
    }

    std::vector<Term> has_many_query(const std::string& parent_model, const std::string& parent_id) const {
        auto m_it = models_.find(parent_model);
        if (m_it == models_.end() || m_it->second.has_many_target.empty()) return {};

        std::string child_model = m_it->second.has_many_target;
        std::string fk_field = m_it->second.foreign_key.empty() ? (parent_model + "_id") : m_it->second.foreign_key;

        return where(child_model, fk_field, parent_id);
    }

    std::optional<Term> belongs_to_query(const std::string& child_model, const std::string& child_id) const {
        auto m_it = models_.find(child_model);
        if (m_it == models_.end() || m_it->second.belongs_to_target.empty()) return std::nullopt;

        auto child_rec = find(child_model, child_id);
        if (!child_rec.has_value()) return std::nullopt;

        std::string parent_model = m_it->second.belongs_to_target;
        std::string fk_field = m_it->second.foreign_key.empty() ? (parent_model + "_id") : m_it->second.foreign_key;

        std::string parent_id;
        if (auto* list_val = child_rec->get_if<garuda::beam::List>()) {
            for (const auto& item : seq_items(list_val->items)) {
                if (auto* tup = item.get_if<garuda::beam::Tuple>()) {
                    const auto& pair = seq_items(tup->items);
                    if (pair.size() == 2) {
                        std::string k = pair[0].get_if<std::string>() ? *pair[0].get_if<std::string>() : to_string(pair[0]);
                        if (k == fk_field) {
                            parent_id = pair[1].get_if<std::string>() ? *pair[1].get_if<std::string>() : to_string(pair[1]);
                            break;
                        }
                    }
                }
            }
        }

        if (parent_id.empty()) return std::nullopt;
        return find(parent_model, parent_id);
    }

    bool migrate_add_column(const std::string& model_name, const std::string& field_name, const std::string& type) {
        auto it = models_.find(model_name);
        if (it == models_.end()) return false;

        it->second.fields[field_name] = type;
        schema_version_++;
        return true;
    }

    std::size_t schema_version() const { return schema_version_; }

private:
    Database& db_;
    std::map<std::string, ModelDef> models_;
    std::size_t schema_version_ = 1;
};

} // namespace garuda::beam::stdlib
