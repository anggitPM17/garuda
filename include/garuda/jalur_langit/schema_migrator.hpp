#pragma once
#include <string>
#include <vector>
#include <functional>
#include "jalur_langit_ultimate.hpp"

namespace garuda::jalur_langit {
    class SchemaMigrator {
    public:
        using MigrationFn = std::function<void(JalurLangitUltimate&)>;

        void add_migration(int version, const std::string& desc, MigrationFn fn) {
            migrations_.push_back({version, desc, fn});
        }

        void run_migrations(JalurLangitUltimate& db) {
            for (auto& m : migrations_) {
                m.fn(db);
                version_ = m.version;
            }
        }

        int current_version() const { return version_; }

    private:
        struct Migration {
            int version;
            std::string desc;
            MigrationFn fn;
        };
        int version_{0};
        std::vector<Migration> migrations_;
    };
}
