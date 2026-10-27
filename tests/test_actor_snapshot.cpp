#include "../include/garuda/beam/gvm/actor_snapshot.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>

using namespace garuda::beam;
namespace fs = std::filesystem;

#define GARUDA_CHECK(cond, msg) \
    if (!(cond)) { \
        std::cerr << "  ❌ ASSERTION FAILED: " << msg << " (" << #cond << ")\n"; \
        std::exit(1); \
    }

void run_test_with_timeout(const std::string& test_name, std::function<void()> test_fn, int timeout_seconds = 5) {
    auto future = std::async(std::launch::async, test_fn);
    if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
        std::cerr << "  ❌ " << test_name << " FAILED (TIMEOUT > " << timeout_seconds << "s)\n";
        std::exit(1); \
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA ACTOR PERSISTENCE & EVENT SOURCING SUITE\n";
    std::cout << "=======================================================\n\n";

    fs::path tmp_base_dir = fs::temp_directory_path() / "garuda_actor_persist_test";
    fs::remove_all(tmp_base_dir);
    fs::create_directories(tmp_base_dir);

    std::string actor_id = "actor_counter_1";

    // === Test 1: Term <-> JSON Serializer Equivalence ===
    run_test_with_timeout("Test 1: Term <-> JSON Serializer Equivalence", []() {
        Term orig_int = Term(42LL);
        std::string json_int = TermJson::serialize(orig_int);
        auto dec_int = TermJson::deserialize(json_int);
        GARUDA_CHECK(dec_int.has_value() && *dec_int->get_if<std::int64_t>() == 42, "Integer serialization must match");

        Term orig_str = Term("Garuda Sourcing");
        std::string json_str = TermJson::serialize(orig_str);
        auto dec_str = TermJson::deserialize(json_str);
        GARUDA_CHECK(dec_str.has_value() && *dec_str->get_if<std::string>() == "Garuda Sourcing", "String serialization must match");

        std::cout << "  ✓ Test 1: Term <-> JSON Serializer Equivalence PASS\n";
    }, 5);

    // === Test 2: Save Snapshot & File Existence ===
    run_test_with_timeout("Test 2: Save Snapshot & File Existence", [&]() {
        Term initial_state = Term(0LL);
        bool save_ok = ActorSnapshot::save_snapshot(actor_id, initial_state, tmp_base_dir.string());
        GARUDA_CHECK(save_ok, "save_snapshot must return true");

        fs::path snap_file = ActorSnapshot::get_snapshot_path(actor_id, tmp_base_dir.string());
        GARUDA_CHECK(fs::exists(snap_file), "Snapshot file must exist on disk");

        std::cout << "  ✓ Test 2: Save Snapshot & File Existence PASS\n";
    }, 5);

    // === Test 3: Append Events & Event Log File Existence ===
    run_test_with_timeout("Test 3: Append Events & Event Log File Existence", [&]() {
        for (int i = 1; i <= 5; ++i) {
            Term event = tuple_of({atom("inc"), Term(1LL)});
            bool app_ok = ActorSnapshot::append_event(actor_id, event, tmp_base_dir.string());
            GARUDA_CHECK(app_ok, "append_event must return true");
        }

        fs::path evlog_file = ActorSnapshot::get_evlog_path(actor_id, tmp_base_dir.string());
        GARUDA_CHECK(fs::exists(evlog_file), "Event log file must exist on disk");

        std::cout << "  ✓ Test 3: Append Events & Event Log File Existence PASS\n";
    }, 5);

    // === Test 4: Simulated Restart, Load Snapshot & Replay Events (Counter = 5) ===
    run_test_with_timeout("Test 4: Simulated Restart & Event Replay (Final = 5)", [&]() {
        // Discard in-memory state, load from disk
        auto snap_state_opt = ActorSnapshot::load_snapshot(actor_id, tmp_base_dir.string());
        GARUDA_CHECK(snap_state_opt.has_value(), "load_snapshot must succeed");
        GARUDA_CHECK(*snap_state_opt->get_if<std::int64_t>() == 0, "Loaded snapshot state must be 0");

        // User-defined apply function
        auto apply_fn = [](const Term& event, const Term& current_state) -> Term {
            std::int64_t curr_val = current_state.get_if<std::int64_t>() ? *current_state.get_if<std::int64_t>() : 0;
            if (auto* lst = event.get_if<List>()) {
                const auto& items = seq_items(lst->items);
                if (items.size() >= 2) {
                    std::string action;
                    if (auto* s = items[0].get_if<std::string>()) action = *s;
                    else if (auto* a = items[0].get_if<Atom>()) action = a->name;

                    if (action == "inc") {
                        std::int64_t delta = items[1].get_if<std::int64_t>() ? *items[1].get_if<std::int64_t>() : 0;
                        return Term(curr_val + delta);
                    }
                }
            } else if (auto* tup = event.get_if<Tuple>()) {
                const auto& items = seq_items(tup->items);
                if (items.size() >= 2) {
                    std::string action;
                    if (auto* s = items[0].get_if<std::string>()) action = *s;
                    else if (auto* a = items[0].get_if<Atom>()) action = a->name;

                    if (action == "inc") {
                        std::int64_t delta = items[1].get_if<std::int64_t>() ? *items[1].get_if<std::int64_t>() : 0;
                        return Term(curr_val + delta);
                    }
                }
            }
            return current_state;
        };

        Term final_state = ActorSnapshot::replay_events(actor_id, tmp_base_dir.string(), *snap_state_opt, apply_fn);
        GARUDA_CHECK(final_state.get_if<std::int64_t>() != nullptr, "Final state must be integer");
        GARUDA_CHECK(*final_state.get_if<std::int64_t>() == 5, "Final state counter after 5 events must be 5");

        std::cout << "  ✓ Test 4: Simulated Restart & Event Replay PASS (Final Counter: "
                  << *final_state.get_if<std::int64_t>() << ")\n";
    }, 5);

    // === Test 5: Corrupted Snapshot File Error Handling ===
    run_test_with_timeout("Test 5: Corrupted Snapshot Error Handling", [&]() {
        std::string corrupt_actor_id = "actor_corrupt_99";
        fs::path corrupt_snap_file = ActorSnapshot::get_snapshot_path(corrupt_actor_id, tmp_base_dir.string());

        // Write invalid corrupted JSON content
        std::ofstream ofs(corrupt_snap_file, std::ios::trunc | std::ios::binary);
        ofs << "INVALID_CORRUPTED_JSON{{{";
        ofs.close();

        auto load_opt = ActorSnapshot::load_snapshot(corrupt_actor_id, tmp_base_dir.string());
        GARUDA_CHECK(!load_opt.has_value(), "load_snapshot must return std::nullopt for corrupted file without crashing");

        std::cout << "  ✓ Test 5: Corrupted Snapshot Error Handling PASS\n";
    }, 5);

    // Cleanup
    fs::remove_all(tmp_base_dir);

    std::cout << "\n🎉 ALL ACTOR PERSISTENCE & EVENT SOURCING TESTS PASSED PERFECTLY!\n";
    return 0;
}
