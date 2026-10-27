#pragma once
// Garuda GVM: Actor Persistence Engine - Snapshot & Event Sourcing Log (`actor_snapshot.hpp`)
#include "term_json.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

namespace garuda::beam {

namespace fs = std::filesystem;

class ActorSnapshot {
public:
    static fs::path get_snapshot_path(const std::string& actor_id, const std::string& base_dir) {
        fs::path snap_dir = fs::path(base_dir) / "snapshots";
        fs::create_directories(snap_dir);
        return snap_dir / (actor_id + ".snap.json");
    }

    static fs::path get_evlog_path(const std::string& actor_id, const std::string& base_dir) {
        fs::path evlog_dir = fs::path(base_dir) / "events";
        fs::create_directories(evlog_dir);
        return evlog_dir / (actor_id + ".evlog.jsonl");
    }

    static bool save_snapshot(const std::string& actor_id, const Term& state, const std::string& base_dir) {
        try {
            fs::path snap_path = get_snapshot_path(actor_id, base_dir);
            fs::path tmp_path = snap_path.string() + ".tmp";

            std::string json_data = TermJson::serialize(state);

            std::ofstream ofs(tmp_path, std::ios::trunc | std::ios::binary);
            if (!ofs.is_open()) return false;
            ofs << json_data;
            ofs.close();

            // Atomic file rename
            std::error_code ec;
            fs::rename(tmp_path, snap_path, ec);
            return !ec;
        } catch (...) {
            return false;
        }
    }

    static std::optional<Term> load_snapshot(const std::string& actor_id, const std::string& base_dir) {
        try {
            fs::path snap_path = get_snapshot_path(actor_id, base_dir);
            if (!fs::exists(snap_path)) return std::nullopt;

            std::ifstream ifs(snap_path, std::ios::binary);
            if (!ifs.is_open()) return std::nullopt;

            std::string json_data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();

            return TermJson::deserialize(json_data);
        } catch (...) {
            return std::nullopt;
        }
    }

    static bool append_event(const std::string& actor_id, const Term& event, const std::string& base_dir) {
        try {
            fs::path evlog_path = get_evlog_path(actor_id, base_dir);
            std::string json_line = TermJson::serialize(event);

            std::ofstream ofs(evlog_path, std::ios::app | std::ios::binary);
            if (!ofs.is_open()) return false;

            ofs << json_line << "\n";
            ofs.flush();
            return true;
        } catch (...) {
            return false;
        }
    }

    using ApplyFn = std::function<Term(const Term& event, const Term& current_state)>;

    static Term replay_events(const std::string& actor_id, const std::string& base_dir, Term initial_state, ApplyFn apply_fn) {
        try {
            fs::path evlog_path = get_evlog_path(actor_id, base_dir);
            if (!fs::exists(evlog_path)) return initial_state;

            std::ifstream ifs(evlog_path, std::ios::binary);
            if (!ifs.is_open()) return initial_state;

            Term current_state = initial_state;
            std::string line;
            while (std::getline(ifs, line)) {
                if (line.empty()) continue;
                auto ev_opt = TermJson::deserialize(line);
                if (ev_opt.has_value() && apply_fn) {
                    current_state = apply_fn(*ev_opt, current_state);
                }
            }
            return current_state;
        } catch (...) {
            return initial_state;
        }
    }
};

} // namespace garuda::beam
