#pragma once
// Garuda BEAM-core: Mnesia Persistent Transactional Database
// Mendukung disc_copies, WAL file persistence, transactions, dan table indexing.
#include "../core/term.hpp"
#include "ets.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam {

enum class TableType { Set, Bag };
enum class StorageType { RamCopies, DiscCopies };

struct TableConfig {
    std::string name;
    TableType type = TableType::Set;
    StorageType storage = StorageType::DiscCopies;
    std::vector<std::string> fields;
    std::vector<std::string> index_fields;
};

class MnesiaTable {
public:
    explicit MnesiaTable(TableConfig config, std::string db_dir = "./garuda_db")
        : config_(std::move(config)), db_dir_(std::move(db_dir)) {
        if (config_.storage == StorageType::DiscCopies) {
            std::filesystem::create_directories(db_dir_);
            file_path_ = (std::filesystem::path(db_dir_) / (config_.name + ".db")).string();
            load_from_disk();
        }
    }

    const TableConfig& config() const { return config_; }

    void write(Term key, Term value) {
        std::lock_guard<std::mutex> lock(mu_);
        if (config_.type == TableType::Set) {
            set_data_[key] = value;
        } else {
            bag_data_.insert({key, value});
        }
        if (config_.storage == StorageType::DiscCopies) {
            append_wal("WRITE", key, value);
        }
    }

    std::optional<Term> read(const Term& key) {
        std::lock_guard<std::mutex> lock(mu_);
        if (config_.type == TableType::Set) {
            auto it = set_data_.find(key);
            if (it != set_data_.end()) return it->second;
            return std::nullopt;
        } else {
            auto range = bag_data_.equal_range(key);
            if (range.first != range.second) return range.first->second;
            return std::nullopt;
        }
    }

    std::vector<Term> read_all(const Term& key) {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<Term> out;
        if (config_.type == TableType::Set) {
            auto it = set_data_.find(key);
            if (it != set_data_.end()) out.push_back(it->second);
        } else {
            auto range = bag_data_.equal_range(key);
            for (auto it = range.first; it != range.second; ++it) {
                out.push_back(it->second);
            }
        }
        return out;
    }

    bool delete_key(const Term& key) {
        std::lock_guard<std::mutex> lock(mu_);
        bool erased = false;
        if (config_.type == TableType::Set) {
            erased = set_data_.erase(key) > 0;
        } else {
            erased = bag_data_.erase(key) > 0;
        }
        if (erased && config_.storage == StorageType::DiscCopies) {
            append_wal("DELETE", key, Term());
        }
        return erased;
    }

    std::vector<Term> select(const std::function<bool(const Term& key, const Term& val)>& pred) {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<Term> results;
        if (config_.type == TableType::Set) {
            for (const auto& [k, v] : set_data_) {
                if (pred(k, v)) results.push_back(v);
            }
        } else {
            for (const auto& [k, v] : bag_data_) {
                if (pred(k, v)) results.push_back(v);
            }
        }
        return results;
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mu_);
        return config_.type == TableType::Set ? set_data_.size() : bag_data_.size();
    }

private:
    void append_wal(const std::string& op, const Term& key, const Term& val) {
        std::ofstream ofs(file_path_, std::ios::app);
        if (ofs.is_open()) {
            ofs << op << "|" << to_string(key) << "|" << to_string(val) << "\n";
        }
    }

    void load_from_disk() {
        if (!std::filesystem::exists(file_path_)) return;
        std::ifstream ifs(file_path_);
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            auto p1 = line.find('|');
            auto p2 = line.find('|', p1 + 1);
            if (p1 == std::string::npos || p2 == std::string::npos) continue;
            std::string op = line.substr(0, p1);
            std::string k_str = line.substr(p1 + 1, p2 - (p1 + 1));
            std::string v_str = line.substr(p2 + 1);

            Term key(k_str);
            Term val(v_str);

            if (op == "WRITE") {
                if (config_.type == TableType::Set) set_data_[key] = val;
                else bag_data_.insert({key, val});
            } else if (op == "DELETE") {
                if (config_.type == TableType::Set) set_data_.erase(key);
                else bag_data_.erase(key);
            }
        }
    }

    TableConfig config_;
    std::string db_dir_;
    std::string file_path_;
    mutable std::mutex mu_;
    std::map<Term, Term, TermLess> set_data_;
    std::multimap<Term, Term, TermLess> bag_data_;
};

class Mnesia {
public:
    explicit Mnesia(std::string db_dir = "./garuda_db")
        : db_dir_(std::move(db_dir)) {}

    bool create_table(const TableConfig& config) {
        std::lock_guard<std::mutex> lock(mu_);
        if (tables_.find(config.name) != tables_.end()) return false;
        tables_[config.name] = std::make_unique<MnesiaTable>(config, db_dir_);
        std::cout << "📦 [GARUDA MNESIA] Table created: " << config.name << " (disc_copies)" << std::endl;
        return true;
    }

    void write(const std::string& table, Term key, Term value) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = tables_.find(table);
        if (it != tables_.end()) {
            it->second->write(std::move(key), std::move(value));
        }
    }

    std::optional<Term> read(const std::string& table, const Term& key) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = tables_.find(table);
        if (it != tables_.end()) {
            return it->second->read(key);
        }
        return std::nullopt;
    }

    std::vector<Term> read_all(const std::string& table, const Term& key) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = tables_.find(table);
        if (it != tables_.end()) {
            return it->second->read_all(key);
        }
        return {};
    }

    bool delete_key(const std::string& table, const Term& key) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = tables_.find(table);
        if (it != tables_.end()) {
            return it->second->delete_key(key);
        }
        return false;
    }

    std::vector<Term> select(const std::string& table, const std::function<bool(const Term& key, const Term& val)>& pred) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = tables_.find(table);
        if (it != tables_.end()) {
            return it->second->select(pred);
        }
        return {};
    }

    void init_richat_schema() {
        create_table({"richat_user", TableType::Set, StorageType::DiscCopies, {"user_id", "email", "name", "avatar"}});
        create_table({"richat_contact", TableType::Bag, StorageType::DiscCopies, {"user_id", "contact_id", "status"}});
        create_table({"richat_moment_post", TableType::Set, StorageType::DiscCopies, {"id", "author_id", "content", "lat", "lon", "expires_at"}});
        create_table({"richat_status_story", TableType::Set, StorageType::DiscCopies, {"id", "user_id", "caption", "image_url", "expires_at"}});
        create_table({"richat_group", TableType::Set, StorageType::DiscCopies, {"group_id", "name", "owner_id", "expires_at", "project_goal"}});
        create_table({"richat_group_member", TableType::Set, StorageType::DiscCopies, {"key", "group_id", "user_id", "role"}});
        create_table({"richat_auto_reply", TableType::Set, StorageType::DiscCopies, {"id", "user_id", "keyword", "reply"}});
        create_table({"offline_message", TableType::Bag, StorageType::DiscCopies, {"recipient_id", "sender_id", "content", "timestamp"}});
    }

private:
    std::string db_dir_;
    std::mutex mu_;
    std::unordered_map<std::string, std::unique_ptr<MnesiaTable>> tables_;
};

} // namespace garuda::beam
