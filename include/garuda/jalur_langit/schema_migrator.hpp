#pragma once

#include "jalur_langit_ultimate.hpp"
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <iostream>

namespace garuda::jalur_langit {

struct Migration {
    int version;
    std::string description;
    std::function<void(JalurLangitUltimate&)> up;
};

class SchemaMigrator {
private:
    std::vector<Migration> migrations_;
    int current_version_ = 0;

public:
    void add_migration(int version, const std::string& desc, std::function<void(JalurLangitUltimate&)> up) {
        migrations_.push_back(Migration{version, desc, up});
        std::sort(migrations_.begin(), migrations_.end(), [](const Migration& a, const Migration& b) {
            return a.version < b.version;
        });
    }

    void run_migrations(JalurLangitUltimate& db) {
        for (const auto& m : migrations_) {
            if (m.version > current_version_) {
                std::cout << "[SchemaMigrator] Running migration " << m.version << ": " << m.description << "\n";
                m.up(db);
                current_version_ = m.version;
            }
        }
    }

    int current_version() const {
        return current_version_;
    }
};

} // namespace garuda::jalur_langit
