#pragma once
// Supervisor ala OTP: one_for_one / one_for_all / rest_for_one, restart permanent/transient/
// temporary, dan batas intensitas restart (max_restarts dalam period_ms).
#include "../core/runtime.hpp"

#include <deque>

namespace garuda::beam {

enum class Restart { Permanent, Transient, Temporary };
enum class Strategy { OneForOne, OneForAll, RestForOne };

struct ChildSpec {
    std::string id;
    std::function<Pid(Process& supervisor)> start;  // gunakan sup.spawn_link(...)
    Restart restart = Restart::Permanent;
};

struct SupervisorFlags {
    Strategy strategy = Strategy::OneForOne;
    unsigned max_restarts = 3;
    std::uint64_t period_ms = 5000;
};

namespace detail {
struct SupState {
    SupervisorFlags flags;
    std::vector<ChildSpec> specs;
    std::vector<Pid> pids;
    std::deque<std::uint64_t> restarts;
    bool started = false;
};

inline void sup_kill_from(Process& sup, SupState& st, std::size_t from_idx) {
    for (std::size_t j = from_idx; j < st.pids.size(); ++j) {
        Pid c = st.pids[j];
        st.pids[j] = Pid{};   // dikosongkan dulu agar EXIT 'killed' diabaikan
        if (c.valid()) sup.exit(c, atom("kill"));
    }
}
inline void sup_start_from(Process& sup, SupState& st, std::size_t from_idx) {
    for (std::size_t j = from_idx; j < st.specs.size(); ++j)
        if (!st.pids[j].valid()) st.pids[j] = st.specs[j].start(sup);
}
}  // namespace detail

inline Body supervisor_body(SupervisorFlags flags, std::vector<ChildSpec> specs) {
    auto st = std::make_shared<detail::SupState>();
    st->flags = flags;
    st->specs = std::move(specs);
    return [st](Process& p) -> Status {
        auto& s = *st;
        if (!s.started) {
            s.started = true;
            p.set_trap_exit(true);
            s.pids.assign(s.specs.size(), Pid{});
            detail::sup_start_from(p, s, 0);
        }
        auto m = p.receive([](const Message& msg) {
            return tuple_size(msg.body) == 3 && is_atom(*tuple_at(msg.body, 0), "EXIT");
        });
        if (!m) return Status::Wait;
        const Pid* who = tuple_at(m->body, 1)->get_if<Pid>();
        const Term reason = *tuple_at(m->body, 2);
        if (!who) return Status::Continue;
        // sinyal dari parent atau dari luar (pid tak valid): matikan seluruh pohon anak lalu berhenti
        if (!who->valid() || *who == p.parent()) { detail::sup_kill_from(p, s, 0); return p.stop(reason); }
        std::size_t i = s.pids.size();
        for (std::size_t j = 0; j < s.pids.size(); ++j) if (s.pids[j] == *who) { i = j; break; }
        if (i == s.pids.size()) return Status::Continue;
        s.pids[i] = Pid{};
        const ChildSpec& spec = s.specs[i];
        bool normal = is_atom(reason, "normal") || is_atom(reason, "shutdown");
        bool restart = spec.restart == Restart::Permanent || (spec.restart == Restart::Transient && !normal);
        if (!restart) return Status::Continue;
        std::uint64_t now = p.runtime().now_ms();
        s.restarts.push_back(now);
        while (!s.restarts.empty() && s.restarts.front() + s.flags.period_ms < now) s.restarts.pop_front();
        if (s.restarts.size() > s.flags.max_restarts) {
            detail::sup_kill_from(p, s, 0);
            return p.stop(atom("shutdown"));
        }
        switch (s.flags.strategy) {
            case Strategy::OneForOne: detail::sup_start_from(p, s, i); break;
            case Strategy::OneForAll: detail::sup_kill_from(p, s, 0); detail::sup_start_from(p, s, 0); break;
            case Strategy::RestForOne:
                detail::sup_kill_from(p, s, i + 1);
                detail::sup_start_from(p, s, i);
                break;
        }
        return Status::Continue;
    };
}

inline Pid start_supervisor(Runtime& rt, SupervisorFlags flags, std::vector<ChildSpec> specs, SpawnOptions opts = {}) {
    return rt.spawn(supervisor_body(flags, std::move(specs)), std::move(opts));
}
inline Pid start_supervisor_link(Process& parent, SupervisorFlags flags, std::vector<ChildSpec> specs, SpawnOptions opts = {}) {
    return parent.spawn_link(supervisor_body(flags, std::move(specs)), std::move(opts));
}

}  // namespace garuda::beam
