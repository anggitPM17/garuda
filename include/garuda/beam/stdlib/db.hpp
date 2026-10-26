#pragma once
// Garuda Standard Library: Native Database Adapter (`db.hpp`)
#include "storage_engine.hpp"

namespace garuda::beam::stdlib {

class Database {
public:
    Database() = default;
    explicit Database(const std::string& path) : engine_(path) {}

    bool open(const std::string& path) {
        return engine_.open(path);
    }

    bool insert(const std::string& table, const std::string& key, const Term& record) {
        return engine_.insert(table, key, record);
    }

    std::optional<Term> get(const std::string& table, const std::string& key) const {
        return engine_.get(table, key);
    }

    std::vector<Term> query_all(const std::string& table) const {
        return engine_.query_all(table);
    }

    bool update(const std::string& table, const std::string& key, const Term& new_record) {
        return engine_.update(table, key, new_record);
    }

    bool remove(const std::string& table, const std::string& key) {
        return engine_.remove(table, key);
    }

    void begin_transaction() {
        engine_.begin_transaction();
    }

    void commit() {
        engine_.commit();
    }

    void rollback() {
        engine_.rollback();
    }

    bool flush() {
        return engine_.checkpoint();
    }

    std::size_t count(const std::string& table) const {
        return engine_.count(table);
    }

private:
    StorageEngine engine_;
};

} // namespace garuda::beam::stdlib
