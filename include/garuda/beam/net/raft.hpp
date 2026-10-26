#pragma once
// Garuda Distributed Engine: Production Raft Consensus Algorithm (`raft.hpp`)
// Mengimplementasikan Pemilihan Pemimpin (Leader Election), Replikasi Log Konsensus,
// Persistence Disk, dan Fault-Tolerant State Machine per-node.
#include "../core/runtime.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam {

enum class RaftRole { Follower, Candidate, Leader };

struct LogEntry {
    std::uint64_t term = 0;
    std::string command;
};

struct RaftPersistentState {
    std::uint64_t current_term = 0;
    std::string voted_for = "";
    std::vector<LogEntry> log;
};

class RaftNode {
public:
    using SendCallback = std::function<void(const std::string& target, const std::string& msg)>;

    RaftNode(std::string node_id, std::vector<std::string> peers)
        : node_id_(std::move(node_id)), peers_(std::move(peers)) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::uint32_t> dis(150, 300);
        election_timeout_ms_ = dis(gen);
        last_heartbeat_ = std::chrono::steady_clock::now();
        for (const auto& p : peers_) {
            peer_last_seen_[p] = std::chrono::steady_clock::now(); // Freshly started peer is initialized ALIVE
        }
    }

    const std::string& node_id() const { return node_id_; }
    const std::vector<std::string>& peers() const { std::lock_guard<std::recursive_mutex> lock(mu_); return peers_; }

    RaftRole role() const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        return role_;
    }

    std::uint64_t current_term() const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        return persistent_.current_term;
    }

    std::uint64_t term() const {
        return current_term();
    }

    std::string voted_for() const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        return persistent_.voted_for;
    }

    std::size_t log_size() const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        return persistent_.log.size();
    }

    std::uint64_t commit_index() const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        return commit_index_;
    }

    void set_send_callback(SendCallback cb) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        send_cb_ = std::move(cb);
    }

    using ApplyCallback = std::function<void(const std::string& command)>;

    void set_apply_callback(ApplyCallback cb) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        apply_cb_ = std::move(cb);
    }

    std::size_t election_count() const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        return election_count_;
    }

    void tick_timer(std::size_t elapsed_ms) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        if (role_ == RaftRole::Leader) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_heartbeat_).count();
        if (elapsed >= static_cast<std::int64_t>(election_timeout_ms_) || elapsed_ms >= election_timeout_ms_) {
            start_election();
        }
    }

    void start_election() {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        if (role_ == RaftRole::Leader) return;

        election_count_++;
        role_ = RaftRole::Candidate;
        persistent_.current_term++;
        persistent_.voted_for = node_id_;
        reset_election_timer();

        std::size_t votes_granted = 1; // Vote for self
        std::size_t total_nodes = peers_.size() + 1;
        std::size_t majority = (total_nodes / 2) + 1;

        std::cout << "🗳️ [GARUDA RAFT] Node " << node_id_ << " started Leader Election for Term " << persistent_.current_term << std::endl;

        if (votes_granted >= majority) {
            become_leader();
            return;
        }

        // Request votes from peers
        if (send_cb_) {
            std::size_t last_log_idx = persistent_.log.size();
            std::uint64_t last_log_term = last_log_idx > 0 ? persistent_.log.back().term : 0;

            for (const auto& peer : peers_) {
                std::ostringstream ss;
                ss << "REQUEST_VOTE " << persistent_.current_term << " " << node_id_ << " " << last_log_idx << " " << last_log_term;
                send_cb_(peer, ss.str());
            }
        }
    }

    void receive_request_vote(std::uint64_t term, const std::string& candidate_id, std::size_t last_log_idx, std::uint64_t last_log_term) {
        std::lock_guard<std::recursive_mutex> lock(mu_);

        if (term > persistent_.current_term) {
            become_follower(term);
        }

        bool vote_granted = false;
        if (term == persistent_.current_term && (persistent_.voted_for.empty() || persistent_.voted_for == candidate_id)) {
            std::size_t my_log_idx = persistent_.log.size();
            std::uint64_t my_log_term = my_log_idx > 0 ? persistent_.log.back().term : 0;

            if (last_log_term > my_log_term || (last_log_term == my_log_term && last_log_idx >= my_log_idx)) {
                vote_granted = true;
                persistent_.voted_for = candidate_id;
                reset_election_timer();
            }
        }

        if (send_cb_) {
            std::ostringstream ss;
            ss << "VOTE_RESPONSE " << persistent_.current_term << " " << (vote_granted ? "1" : "0") << " " << node_id_;
            send_cb_(candidate_id, ss.str());
        }
    }

    void receive_vote_response(std::uint64_t term, bool vote_granted, const std::string& from_node) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        if (role_ != RaftRole::Candidate || term != persistent_.current_term) return;

        if (vote_granted) {
            votes_received_.insert(from_node);
            votes_received_.insert(node_id_);

            std::size_t total_nodes = peers_.size() + 1;
            std::size_t majority = (total_nodes / 2) + 1;

            if (votes_received_.size() >= majority) {
                become_leader();
            }
        }
    }

    bool append_command(const std::string& command) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        if (role_ != RaftRole::Leader) return false;

        persistent_.log.push_back(LogEntry{persistent_.current_term, command});
        std::size_t new_idx = persistent_.log.size();

        // Replicate command to peers
        replicate_log_entries();

        // Leader commits immediately if majority confirmed
        std::size_t replicated_count = 1; // Self
        for (const auto& peer : peers_) {
            if (match_index_[peer] >= new_idx) replicated_count++;
        }

        std::size_t total_nodes = peers_.size() + 1;
        if (replicated_count >= (total_nodes / 2) + 1) {
            commit_index_ = new_idx;
            if (apply_cb_) apply_cb_(command);
        }

        return true;
    }

    bool append_entry(std::uint64_t term, Term command) {
        (void)term;
        return append_command(to_string(command));
    }

    void receive_append_entries(std::uint64_t term, const std::string& leader_id, std::size_t prev_log_idx, std::uint64_t prev_log_term, const std::vector<LogEntry>& entries, std::size_t leader_commit) {
        std::lock_guard<std::recursive_mutex> lock(mu_);

        if (term < persistent_.current_term) {
            if (send_cb_) {
                std::ostringstream ss;
                ss << "APPEND_RESPONSE " << persistent_.current_term << " 0 " << persistent_.log.size() << " " << node_id_;
                send_cb_(leader_id, ss.str());
            }
            return;
        }

        become_follower(term);
        reset_election_timer();

        bool success = true;
        if (prev_log_idx > 0) {
            if (persistent_.log.size() < prev_log_idx || persistent_.log[prev_log_idx - 1].term != prev_log_term) {
                success = false;
            }
        }

        if (success) {
            for (const auto& entry : entries) {
                persistent_.log.push_back(entry);
                if (apply_cb_) apply_cb_(entry.command);
            }
            if (leader_commit > commit_index_) {
                commit_index_ = (std::min)(leader_commit, persistent_.log.size());
            }
        }

        if (send_cb_) {
            std::ostringstream ss;
            ss << "APPEND_RESPONSE " << persistent_.current_term << " " << (success ? "1" : "0") << " " << persistent_.log.size() << " " << node_id_;
            send_cb_(leader_id, ss.str());
        }
    }

    void receive_append_response(std::uint64_t term, bool success, std::size_t match_idx, const std::string& from_node) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        if (role_ != RaftRole::Leader || term != persistent_.current_term) return;

        if (success) {
            match_index_[from_node] = match_idx;
            next_index_[from_node] = match_idx + 1;

            // Update leader commit index based on majority match index
            std::vector<std::size_t> matches = {persistent_.log.size()};
            for (const auto& peer : peers_) {
                matches.push_back(match_index_[peer]);
            }
            std::sort(matches.begin(), matches.end());
            std::size_t majority_commit = matches[matches.size() / 2];
            if (majority_commit > commit_index_) {
                commit_index_ = majority_commit;
            }
        }
    }

    void save_state(const std::string& filepath) const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        std::ofstream ofs(filepath, std::ios::binary);
        if (!ofs.is_open()) return;

        ofs << "RAFT_STATE_V1\n";
        ofs << "TERM:" << persistent_.current_term << "\n";
        ofs << "VOTED:" << persistent_.voted_for << "\n";
        ofs << "LOG_SIZE:" << persistent_.log.size() << "\n";
        for (const auto& entry : persistent_.log) {
            ofs << entry.term << " " << entry.command << "\n";
        }
        ofs.close();
    }

    bool load_state(const std::string& filepath) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        std::ifstream ifs(filepath, std::ios::binary);
        if (!ifs.is_open()) return false;

        std::string header;
        std::getline(ifs, header);
        if (header != "RAFT_STATE_V1") return false;

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.rfind("TERM:", 0) == 0) {
                persistent_.current_term = std::stoull(line.substr(5));
            } else if (line.rfind("VOTED:", 0) == 0) {
                persistent_.voted_for = line.substr(6);
            } else if (line.rfind("LOG_SIZE:", 0) == 0) {
                std::size_t count = std::stoull(line.substr(9));
                persistent_.log.clear();
                for (std::size_t i = 0; i < count && std::getline(ifs, line); ++i) {
                    std::istringstream ss(line);
                    std::uint64_t entry_term;
                    std::string cmd;
                    ss >> entry_term >> cmd;
                    persistent_.log.push_back(LogEntry{entry_term, cmd});
                }
            }
        }
        return true;
    }

    bool take_snapshot(std::size_t last_included_index, const std::string& snapshot_data) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        if (last_included_index > persistent_.log.size()) return false;

        snapshot_last_index_ = last_included_index;
        if (last_included_index > 0 && !persistent_.log.empty()) {
            snapshot_last_term_ = persistent_.log[last_included_index - 1].term;
            persistent_.log.erase(persistent_.log.begin(), persistent_.log.begin() + last_included_index);
        }
        snapshot_bytes_ = snapshot_data;
        return true;
    }

    bool install_snapshot(std::size_t last_included_index, std::uint64_t last_included_term, const std::string& snapshot_data) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        snapshot_last_index_ = last_included_index;
        snapshot_last_term_ = last_included_term;
        snapshot_bytes_ = snapshot_data;
        persistent_.log.clear();
        commit_index_ = last_included_index;
        return true;
    }

    std::vector<std::string> detect_failure(std::size_t heartbeat_timeout_ms = 500) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        std::vector<std::string> dead_nodes;
        auto now = std::chrono::steady_clock::now();

        for (const auto& [peer, last_seen] : peer_last_seen_) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_seen).count();
            if (elapsed >= static_cast<std::int64_t>(heartbeat_timeout_ms)) {
                dead_nodes.push_back(peer);
            }
        }
        return dead_nodes;
    }

    bool save_state_to_disk(const std::string& filepath) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        std::ofstream ofs(filepath, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open()) return false;

        ofs << persistent_.current_term << "\n"
            << persistent_.voted_for << "\n"
            << persistent_.log.size() << "\n";

        for (const auto& entry : persistent_.log) {
            ofs << entry.term << " " << entry.command << "\n";
        }
        return true;
    }

    bool load_state_from_disk(const std::string& filepath) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        std::ifstream ifs(filepath, std::ios::binary);
        if (!ifs.is_open()) return false;

        std::size_t log_size = 0;
        if (ifs >> persistent_.current_term >> persistent_.voted_for >> log_size) {
            persistent_.log.clear();
            for (std::size_t i = 0; i < log_size; ++i) {
                std::uint64_t term;
                std::string cmd;
                if (ifs >> term >> cmd) {
                    persistent_.log.push_back(LogEntry{term, cmd});
                }
            }
            return true;
        }
        return false;
    }

    std::size_t snapshot_last_index() const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        return snapshot_last_index_;
    }

    const std::string& snapshot_bytes() const {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        return snapshot_bytes_;
    }

    void add_peer(const std::string& peer_id) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        if (std::find(peers_.begin(), peers_.end(), peer_id) == peers_.end()) {
            peers_.push_back(peer_id);
            next_index_[peer_id] = persistent_.log.size() + 1;
            match_index_[peer_id] = 0;
            peer_last_seen_[peer_id] = std::chrono::steady_clock::now();
        }
    }

    void remove_peer(const std::string& peer_id) {
        std::lock_guard<std::recursive_mutex> lock(mu_);
        peers_.erase(std::remove(peers_.begin(), peers_.end(), peer_id), peers_.end());
        next_index_.erase(peer_id);
        match_index_.erase(peer_id);
        peer_last_seen_.erase(peer_id);
    }

    void process_network_message(const std::string& msg) {
        std::istringstream ss(msg);
        std::string type;
        ss >> type;

        if (type == "REQUEST_VOTE") {
            std::uint64_t term, last_log_term;
            std::size_t last_log_idx;
            std::string cand_id;
            ss >> term >> cand_id >> last_log_idx >> last_log_term;
            {
                std::lock_guard<std::recursive_mutex> lock(mu_);
                peer_last_seen_[cand_id] = std::chrono::steady_clock::now();
            }
            receive_request_vote(term, cand_id, last_log_idx, last_log_term);
        } else if (type == "VOTE_RESPONSE") {
            std::uint64_t term;
            bool granted;
            std::string from;
            ss >> term >> granted >> from;
            {
                std::lock_guard<std::recursive_mutex> lock(mu_);
                peer_last_seen_[from] = std::chrono::steady_clock::now();
            }
            receive_vote_response(term, granted, from);
        } else if (type == "APPEND_ENTRIES") {
            std::uint64_t term, prev_log_term;
            std::size_t prev_log_idx, commit_idx;
            std::string leader_id, cmd;
            if (ss >> term >> leader_id >> prev_log_idx >> prev_log_term >> commit_idx) {
                std::getline(ss, cmd);
                if (!cmd.empty() && cmd.front() == ' ') cmd.erase(0, 1);
            }
            {
                std::lock_guard<std::recursive_mutex> lock(mu_);
                peer_last_seen_[leader_id] = std::chrono::steady_clock::now();
            }
            std::vector<LogEntry> entries;
            if (!cmd.empty()) entries.push_back(LogEntry{term, cmd});
            receive_append_entries(term, leader_id, prev_log_idx, prev_log_term, entries, commit_idx);
        } else if (type == "APPEND_RESPONSE") {
            std::uint64_t term;
            bool success;
            std::size_t match_idx;
            std::string from;
            ss >> term >> success >> match_idx >> from;
            {
                std::lock_guard<std::recursive_mutex> lock(mu_);
                peer_last_seen_[from] = std::chrono::steady_clock::now();
            }
            receive_append_response(term, success, match_idx, from);
        }
    }

private:
    void become_follower(std::uint64_t term) {
        role_ = RaftRole::Follower;
        persistent_.current_term = term;
        persistent_.voted_for.clear();
        votes_received_.clear();
    }

    void become_leader() {
        role_ = RaftRole::Leader;
        std::cout << "👑 [GARUDA RAFT] Node " << node_id_ << " ELECTED LEADER for Term " << persistent_.current_term << std::endl;
        for (const auto& peer : peers_) {
            next_index_[peer] = persistent_.log.size() + 1;
            match_index_[peer] = 0;
        }
        replicate_log_entries();
    }

    void reset_election_timer() {
        last_heartbeat_ = std::chrono::steady_clock::now();
    }

    void replicate_log_entries() {
        if (!send_cb_) return;
        std::size_t prev_log_idx = persistent_.log.size() > 1 ? persistent_.log.size() - 1 : 0;
        std::uint64_t prev_log_term = prev_log_idx > 0 ? persistent_.log[prev_log_idx - 1].term : 0;

        for (const auto& peer : peers_) {
            std::ostringstream ss;
            std::string cmd = persistent_.log.empty() ? "" : persistent_.log.back().command;
            ss << "APPEND_ENTRIES " << persistent_.current_term << " " << node_id_ << " "
               << prev_log_idx << " " << prev_log_term << " " << commit_index_ << " " << cmd;
            send_cb_(peer, ss.str());
        }
    }

    std::string node_id_;
    std::vector<std::string> peers_;
    mutable std::recursive_mutex mu_;
    RaftRole role_ = RaftRole::Follower;
    std::size_t commit_index_ = 0;
    RaftPersistentState persistent_;
    std::map<std::string, std::size_t> next_index_;
    std::map<std::string, std::size_t> match_index_;
    std::map<std::string, std::chrono::steady_clock::time_point> peer_last_seen_;
    std::set<std::string> votes_received_;
    std::chrono::steady_clock::time_point last_heartbeat_;
    std::uint32_t election_timeout_ms_ = 200;
    std::size_t election_count_ = 0;
    std::size_t snapshot_last_index_ = 0;
    std::uint64_t snapshot_last_term_ = 0;
    std::string snapshot_bytes_;
    SendCallback send_cb_;
    ApplyCallback apply_cb_;
};

} // namespace garuda::beam
