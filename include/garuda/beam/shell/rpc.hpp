#pragma once
// =====================================================================
// FASE G6.2 - Garuda RPC: Remote procedure call antar-node
// =====================================================================
#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <optional>
#include <chrono>
#include <vector>
#include <memory>
#include <cstddef>
#include <thread>

namespace garuda::beam {

struct RpcRequest {
    std::string node_id;
    std::string module;
    std::string function;
    std::vector<std::string> args;
    std::chrono::steady_clock::time_point created_at;
};

struct RpcResponse {
    bool success = false;
    std::string result;
    std::string error;
    std::chrono::microseconds duration{0};
};

class RpcServer {
private:
    std::string node_id_;
    std::map<std::string, std::map<std::string, std::function<std::string(const std::vector<std::string>&)>>> modules_;
    mutable std::mutex mu_;

public:
    explicit RpcServer(std::string node_id)
        : node_id_(std::move(node_id)) {}

    void register_function(const std::string& module,
                           const std::string& function,
                           std::function<std::string(const std::vector<std::string>&)> handler) {
        std::lock_guard<std::mutex> lock(mu_);
        modules_[module][function] = std::move(handler);
    }

    RpcResponse handle(const RpcRequest& req) {
        auto t0 = std::chrono::steady_clock::now();
        RpcResponse resp;

        std::function<std::string(const std::vector<std::string>&)> fn_handler;
        {
            std::lock_guard<std::mutex> lock(mu_);
            auto mod_it = modules_.find(req.module);
            if (mod_it != modules_.end()) {
                auto fn_it = mod_it->second.find(req.function);
                if (fn_it != mod_it->second.end()) {
                    fn_handler = fn_it->second;
                }
            }
        }

        if (!fn_handler) {
            resp.success = false;
            resp.error = "badrpc: module_or_function_not_found (" + req.module + ":" + req.function + ")";
            auto t1 = std::chrono::steady_clock::now();
            resp.duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
            return resp;
        }

        try {
            resp.result = fn_handler(req.args);
            resp.success = true;
        } catch (const std::exception& e) {
            resp.success = false;
            resp.error = std::string("badrpc: exception (") + e.what() + ")";
        } catch (...) {
            resp.success = false;
            resp.error = "badrpc: unknown_exception";
        }

        auto t1 = std::chrono::steady_clock::now();
        resp.duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        return resp;
    }

    std::size_t function_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        std::size_t count = 0;
        for (const auto& [mod, fns] : modules_) {
            count += fns.size();
        }
        return count;
    }

    std::size_t module_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return modules_.size();
    }

    std::string node_id() const {
        std::lock_guard<std::mutex> lock(mu_);
        return node_id_;
    }
};

class RpcClient {
private:
    RpcServer* local_server_ = nullptr;
    std::map<std::string, RpcServer*> remote_servers_;
    mutable std::mutex mu_;

public:
    void set_local_server(RpcServer* server) {
        std::lock_guard<std::mutex> lock(mu_);
        local_server_ = server;
    }

    void add_remote_server(const std::string& node_id, RpcServer* server) {
        std::lock_guard<std::mutex> lock(mu_);
        remote_servers_[node_id] = server;
    }

    RpcResponse call(const std::string& node_id,
                     const std::string& module,
                     const std::string& function,
                     const std::vector<std::string>& args) {
        RpcRequest req;
        req.node_id = node_id;
        req.module = module;
        req.function = function;
        req.args = args;
        req.created_at = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(mu_);
        if (local_server_ && local_server_->node_id() == node_id) {
            return local_server_->handle(req);
        }

        auto it = remote_servers_.find(node_id);
        if (it != remote_servers_.end() && it->second) {
            return it->second->handle(req);
        }

        RpcResponse resp;
        resp.success = false;
        resp.error = "badrpc: noconnection (" + node_id + ")";
        return resp;
    }

    std::size_t remote_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return remote_servers_.size();
    }
};

} // namespace garuda::beam
