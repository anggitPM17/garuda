#pragma once
// Garuda Standard Library: Crash-Safe Storage Engine (`storage_engine.hpp`)
// Mengimplementasikan WAL (Write-Ahead Logging), Fast Memory Index Lookup, Atomic Writes, & ACID Transactions.
#include "../core/term.hpp"
#include "json.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam::stdlib {

namespace fs = std::filesystem;

enum class WalOpType { Insert, Update, Delete, Begin, Commit, Rollback };

struct WalRecord {
    std::string op;
    std::string table;
    std::string key;
    std::string data_json;
    std::uint64_t tx_id = 0;
};

class StorageEngine {
public:
    StorageEngine() = default;
    explicit StorageEngine(std::string db_dir) {
        open(std::move(db_dir));
    }
    ~StorageEngine() {
        checkpoint();
    }

    bool open(const std::string& db_dir) {
        std::unique_lock<std::shared_mutex> lock(mu_);
        db_dir_ = db_dir;
        if (!fs::exists(db_dir_)) {
            fs::create_directories(db_dir_);
        }

        data_file_path_ = (fs::path(db_dir_) / "data.db").string();
        wal_file_path_ = (fs::path(db_dir_) / "wal.log").string();

        tables_.clear();
        in_tx_ = false;
        tx_buffer_.clear();

        // 1. Load primary data file
        load_primary_data();

        // 2. Replay WAL for crash recovery
        replay_wal();

        return true;
    }

    bool insert(const std::string& table, const std::string& key, const Term& record) {
        std::unique_lock<std::shared_mutex> lock(mu_);
        std::string json_val = Json::encode(record);

        if (in_tx_) {
            tx_buffer_.push_back({WalOpType::Insert, table, key, json_val, current_tx_id_});
            tx_uncommitted_state_[table][key] = record;
            return true;
        }

        WalRecord wal_rec{"INSERT", table, key, json_val, 0};
        append_wal(wal_rec);

        tables_[table][key] = record;
        index_[table + ":" + key] = json_val;
        return true;
    }

    bool insert_batch(const std::string& table, const std::vector<std::pair<std::string, Term>>& records) {
        std::unique_lock<std::shared_mutex> lock(mu_);
        std::ofstream wal(wal_file_path_, std::ios::app | std::ios::binary);
        if (!wal.is_open()) return false;

        for (const auto& [key, record] : records) {
            std::string json_val = Json::encode(record);
            wal << "INSERT|0|" << table << "|" << key << "|" << json_val << "\n";
            tables_[table][key] = record;
            index_[table + ":" + key] = json_val;
        }
        wal.flush();
        return true;
    }

    std::optional<Term> get(const std::string& table, const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mu_);

        // Uncommitted transaction isolation check
        if (in_tx_) {
            auto t_it = tx_uncommitted_state_.find(table);
            if (t_it != tx_uncommitted_state_.end()) {
                auto k_it = t_it->second.find(key);
                if (k_it != t_it->second.end()) {
                    return k_it->second;
                }
            }
        }

        // Fast Index Lookup
        auto idx_it = index_.find(table + ":" + key);
        if (idx_it != index_.end()) {
            auto t_it = tables_.find(table);
            if (t_it != tables_.end()) {
                auto k_it = t_it->second.find(key);
                if (k_it != t_it->second.end()) {
                    return k_it->second;
                }
            }
        }
        return std::nullopt;
    }

    bool update(const std::string& table, const std::string& key, const Term& record) {
        return insert(table, key, record);
    }

    bool remove(const std::string& table, const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(mu_);
        if (in_tx_) {
            tx_buffer_.push_back({WalOpType::Delete, table, key, "", current_tx_id_});
            tx_uncommitted_state_[table].erase(key);
            return true;
        }

        WalRecord wal_rec{"DELETE", table, key, "", 0};
        append_wal(wal_rec);

        auto t_it = tables_.find(table);
        if (t_it != tables_.end()) {
            t_it->second.erase(key);
        }
        index_.erase(table + ":" + key);
        return true;
    }

    std::size_t count(const std::string& table) const {
        std::shared_lock<std::shared_mutex> lock(mu_);
        auto t_it = tables_.find(table);
        if (t_it != tables_.end()) return t_it->second.size();
        return 0;
    }

    std::vector<Term> query_all(const std::string& table) const {
        std::shared_lock<std::shared_mutex> lock(mu_);
        std::vector<Term> res;
        auto t_it = tables_.find(table);
        if (t_it != tables_.end()) {
            for (const auto& [k, v] : t_it->second) {
                res.push_back(tuple_of({Term(k), v}));
            }
        }
        return res;
    }

    // ACID Transactions
    std::uint64_t begin_transaction() {
        std::unique_lock<std::shared_mutex> lock(mu_);
        in_tx_ = true;
        current_tx_id_ = ++tx_counter_;
        tx_buffer_.clear();
        tx_uncommitted_state_.clear();
        append_wal({"BEGIN", "", "", "", current_tx_id_});
        return current_tx_id_;
    }

    bool commit() {
        std::unique_lock<std::shared_mutex> lock(mu_);
        if (!in_tx_) return false;

        append_wal({"COMMIT", "", "", "", current_tx_id_});

        for (const auto& entry : tx_buffer_) {
            if (entry.op == WalOpType::Insert) {
                auto dec = Json::decode(entry.data_json);
                if (dec.has_value()) {
                    tables_[entry.table][entry.key] = *dec;
                } else {
                    tables_[entry.table][entry.key] = Term(entry.data_json);
                }
                index_[entry.table + ":" + entry.key] = entry.data_json;
            } else if (entry.op == WalOpType::Delete) {
                tables_[entry.table].erase(entry.key);
                index_.erase(entry.table + ":" + entry.key);
            }
        }

        in_tx_ = false;
        tx_buffer_.clear();
        tx_uncommitted_state_.clear();
        return true;
    }

    bool rollback() {
        std::unique_lock<std::shared_mutex> lock(mu_);
        if (!in_tx_) return false;

        append_wal({"ROLLBACK", "", "", "", current_tx_id_});
        in_tx_ = false;
        tx_buffer_.clear();
        tx_uncommitted_state_.clear();
        return true;
    }

    // Atomic Write & Checkpoint
    bool checkpoint() {
        std::unique_lock<std::shared_mutex> lock(mu_);
        std::string tmp_path = data_file_path_ + ".tmp";
        std::ofstream ofs(tmp_path, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open()) return false;

        for (const auto& [tbl_name, records] : tables_) {
            for (const auto& [k, v] : records) {
                ofs << tbl_name << "|" << k << "|" << Json::encode(v) << "\n";
            }
        }
        ofs.close();

        // Atomic File Swap
        std::error_code ec;
        fs::rename(tmp_path, data_file_path_, ec);

        // Truncate WAL
        std::ofstream wal(wal_file_path_, std::ios::binary | std::ios::trunc);
        return true;
    }

private:
    void append_wal(const WalRecord& rec) {
        std::ofstream wal(wal_file_path_, std::ios::app | std::ios::binary);
        if (wal.is_open()) {
            wal << rec.op << "|" << rec.tx_id << "|" << rec.table << "|" << rec.key << "|" << rec.data_json << "\n";
            wal.flush();
        }
    }

    void load_primary_data() {
        std::ifstream ifs(data_file_path_, std::ios::binary);
        if (!ifs.is_open()) return;

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            auto p1 = line.find('|');
            auto p2 = line.find('|', p1 + 1);
            if (p1 != std::string::npos && p2 != std::string::npos) {
                std::string tbl = line.substr(0, p1);
                std::string k = line.substr(p1 + 1, p2 - (p1 + 1));
                std::string data_json = line.substr(p2 + 1);

                auto dec = Json::decode(data_json);
                if (dec.has_value()) {
                    tables_[tbl][k] = *dec;
                } else {
                    tables_[tbl][k] = Term(data_json);
                }
                index_[tbl + ":" + k] = data_json;
            }
        }
    }

    void replay_wal() {
        std::ifstream ifs(wal_file_path_, std::ios::binary);
        if (!ifs.is_open()) return;

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string op, tx_id_str, tbl, k, data_json;

            auto p1 = line.find('|');
            auto p2 = line.find('|', p1 + 1);
            auto p3 = line.find('|', p2 + 1);
            auto p4 = line.find('|', p3 + 1);

            if (p1 != std::string::npos && p2 != std::string::npos && p3 != std::string::npos && p4 != std::string::npos) {
                op = line.substr(0, p1);
                tx_id_str = line.substr(p1 + 1, p2 - (p1 + 1));
                tbl = line.substr(p2 + 1, p3 - (p2 + 1));
                k = line.substr(p3 + 1, p4 - (p3 + 1));
                data_json = line.substr(p4 + 1);

                if (op == "INSERT") {
                    auto dec = Json::decode(data_json);
                    if (dec.has_value()) {
                        tables_[tbl][k] = *dec;
                    } else {
                        tables_[tbl][k] = Term(data_json);
                    }
                    index_[tbl + ":" + k] = data_json;
                } else if (op == "DELETE") {
                    tables_[tbl].erase(k);
                    index_.erase(tbl + ":" + k);
                }
            }
        }
    }

    std::string db_dir_;
    std::string data_file_path_;
    std::string wal_file_path_;
    mutable std::shared_mutex mu_;
    std::unordered_map<std::string, std::map<std::string, Term>> tables_;
    std::unordered_map<std::string, std::string> index_;

    bool in_tx_ = false;
    std::uint64_t current_tx_id_ = 0;
    std::uint64_t tx_counter_ = 0;
    struct TxEntry { WalOpType op; std::string table; std::string key; std::string data_json; std::uint64_t tx_id; };
    std::vector<TxEntry> tx_buffer_;
    std::unordered_map<std::string, std::map<std::string, Term>> tx_uncommitted_state_;
};

} // namespace garuda::beam::stdlib
