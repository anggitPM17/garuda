#pragma once
// Garuda Distributed Engine: Distributed Database Engine (`distributed_db.hpp`)
// Mengimplementasikan Sharding via Consistent Hashing, Replikasi N-Node Quorum, Raft Cross-Shard Consistency, & Query Routing Forwarding.
#include "raft.hpp"
#include "global_registry.hpp"
#include "../stdlib/storage_engine.hpp"
#include "../stdlib/json.hpp"
#include "../stdlib/http_server.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam {

using namespace garuda::beam::stdlib;

struct TableConfig {
    std::string name;
    std::size_t shard_count = 3;
    std::size_t replication_factor = 3;
};

class DistributedDatabase {
public:
    using ForwardCallback = std::function<std::string(const std::string& target_node, const std::string& request)>;

    DistributedDatabase(std::string node_id, RaftNode& raft, StorageEngine& storage)
        : node_id_(std::move(node_id)), raft_(raft), storage_(storage), ring_(256) {
        ring_.add_node(node_id_);
        for (const auto& peer : raft_.peers()) {
            ring_.add_node(peer);
        }

        raft_.set_apply_callback([this](const std::string& command) {
            apply_command(command);
        });
    }

    void set_forward_callback(ForwardCallback cb) {
        std::lock_guard<std::mutex> lock(mu_);
        forward_cb_ = std::move(cb);
    }

    void create_table(const std::string& table, std::size_t shard_count = 3, std::size_t replication_factor = 3) {
        std::lock_guard<std::mutex> lock(mu_);
        tables_[table] = TableConfig{table, shard_count, replication_factor};
    }

    bool insert(const std::string& table, const std::string& key, const Term& value) {
        std::string json_val = Json::encode(value);

        std::string cmd = "DB_INSERT " + table + " " + key + " " + json_val;
        bool ok = raft_.append_command(cmd);
        if (ok) {
            apply_command(cmd);
        }
        return ok;
    }

    bool insert_batch(const std::string& table, const std::vector<std::pair<std::string, Term>>& items) {
        if (raft_.role() != RaftRole::Leader) return false;

        std::ostringstream ss;
        ss << "DB_BATCH " << table << " " << items.size();
        for (const auto& [k, v] : items) {
            ss << " " << k << " " << Json::encode(v);
        }

        std::string cmd = ss.str();
        bool ok = raft_.append_command(cmd);
        if (ok) {
            apply_command(cmd);
        }
        return ok;
    }

    std::optional<Term> get(const std::string& table, const std::string& key) const {
        std::string primary = get_primary_node(key);

        if (primary == node_id_) {
            // Local Primary Read
            return storage_.get(table, key);
        }

        // Check local secondary replica cache first
        auto local_res = storage_.get(table, key);
        if (local_res.has_value()) return local_res;

        // Forward query to Primary Replica Node
        return forward_query(primary, table, key);
    }

    bool remove(const std::string& table, const std::string& key) {
        std::string cmd = "DB_DELETE " + table + " " + key;
        bool ok = raft_.append_command(cmd);
        if (ok) {
            apply_command(cmd);
        }
        return ok;
    }

    std::string get_primary_node(const std::string& key) const {
        return ring_.get_node(key);
    }

    std::vector<std::string> get_replica_nodes(const std::string& key, std::size_t n = 3) const {
        return ring_.get_replica_nodes(key, n);
    }

    void on_node_join(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(mu_);
        ring_.add_node(node_id);
        raft_.add_peer(node_id);
    }

    void on_node_leave(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(mu_);
        ring_.remove_node(node_id);
        raft_.remove_peer(node_id);
    }

    void apply_command(const std::string& command) {
        std::istringstream ss(command);
        std::string action, table;
        if (ss >> action >> table) {
            if (action == "DB_BATCH") {
                std::size_t batch_size = 0;
                ss >> batch_size;
                for (std::size_t i = 0; i < batch_size; ++i) {
                    std::string k, v_json;
                    if (ss >> k >> v_json) {
                        auto dec = Json::decode(v_json);
                        if (dec.has_value()) {
                            storage_.insert(table, k, *dec);
                        } else {
                            storage_.insert(table, k, Term(v_json));
                        }
                    }
                }
            } else {
                std::string key, data_json;
                if (ss >> key) {
                    std::getline(ss, data_json);
                    if (!data_json.empty() && data_json.front() == ' ') data_json.erase(0, 1);

                    if (action == "DB_INSERT") {
                        auto dec = Json::decode(data_json);
                        if (dec.has_value()) {
                            storage_.insert(table, key, *dec);
                        } else {
                            storage_.insert(table, key, Term(data_json));
                        }
                    } else if (action == "DB_DELETE") {
                        storage_.remove(table, key);
                    }
                }
            }
        }
    }

    void register_http_endpoints(garuda::beam::stdlib::Router& router) {
        router.post("/db/insert", [this](const HttpRequest& req) {
            auto dec = Json::decode(req.body);
            std::string table = "default", key = "k1", val_str = "v1";

            if (dec.has_value()) {
                if (auto* list_val = dec->get_if<garuda::beam::List>()) {
                    for (const auto& item : seq_items(list_val->items)) {
                        if (auto* tup = item.get_if<garuda::beam::Tuple>()) {
                            const auto& pair = seq_items(tup->items);
                            if (pair.size() == 2 && pair[0].get_if<std::string>()) {
                                std::string k = *pair[0].get_if<std::string>();
                                std::string v = pair[1].get_if<std::string>() ? *pair[1].get_if<std::string>() : to_string(pair[1]);
                                if (k == "table") table = v;
                                if (k == "key") key = v;
                                if (k == "value") val_str = v;
                            }
                        }
                    }
                }
            }

            bool ok = insert(table, key, Term(val_str));
            if (ok) {
                return HttpServer::make_json("{\"status\":\"ok\",\"table\":\"" + table + "\",\"key\":\"" + key + "\"}");
            }
            return HttpServer::make_not_found("{\"error\":\"insert_failed\"}");
        });

        router.post("/db/query", [this](const HttpRequest& req) {
            auto dec = Json::decode(req.body);
            std::string table = "default", key = "k1";

            if (dec.has_value()) {
                if (auto* list_val = dec->get_if<garuda::beam::List>()) {
                    for (const auto& item : seq_items(list_val->items)) {
                        if (auto* tup = item.get_if<garuda::beam::Tuple>()) {
                            const auto& pair = seq_items(tup->items);
                            if (pair.size() == 2 && pair[0].get_if<std::string>()) {
                                std::string k = *pair[0].get_if<std::string>();
                                std::string v = pair[1].get_if<std::string>() ? *pair[1].get_if<std::string>() : to_string(pair[1]);
                                if (k == "table") table = v;
                                if (k == "key") key = v;
                            }
                        }
                    }
                }
            }

            auto res = get(table, key);
            if (res.has_value()) {
                std::string val = res->get_if<std::string>() ? *res->get_if<std::string>() : to_string(*res);
                return HttpServer::make_json("{\"status\":\"ok\",\"table\":\"" + table + "\",\"key\":\"" + key + "\",\"value\":\"" + val + "\"}");
            }
            return HttpServer::make_not_found("{\"error\":\"key_not_found\"}");
        });
    }

private:
    std::optional<Term> forward_query(const std::string& primary_node, const std::string& table, const std::string& key) const {
        if (forward_cb_) {
            std::string req_msg = "DB_FORWARD_READ " + table + " " + key;
            std::string resp_str = forward_cb_(primary_node, req_msg);
            if (!resp_str.empty() && resp_str != "NOT_FOUND") {
                auto dec = Json::decode(resp_str);
                if (dec.has_value()) return *dec;
                return Term(resp_str);
            }
        }
        return storage_.get(table, key);
    }

    std::string node_id_;
    RaftNode& raft_;
    StorageEngine& storage_;
    ConsistentHashRing ring_;
    mutable std::mutex mu_;
    ForwardCallback forward_cb_;
    std::map<std::string, TableConfig> tables_;
};

} // namespace garuda::beam
