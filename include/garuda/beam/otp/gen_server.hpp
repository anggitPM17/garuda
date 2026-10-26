#pragma once
// gen_server ala OTP: call (balasan dengan Ref), cast, info, terminate, stop.
#include "../core/runtime.hpp"

namespace garuda::beam {

struct CallResult {
    Term reply;
    std::optional<Term> stop;  // diisi = server berhenti dengan alasan ini setelah membalas
};

class GenServer {
public:
    virtual ~GenServer() = default;
    virtual void init(Process&) {}
    virtual CallResult handle_call(Process&, const Term& request, Pid from) = 0;
    virtual void handle_cast(Process&, const Term&) {}
    virtual void handle_info(Process&, const Term&) {}
    virtual void terminate(Process&, const Term&) {}
};

inline Body gen_server_body(std::shared_ptr<GenServer> srv) {
    auto inited = std::make_shared<bool>(false);
    return [srv, inited](Process& p) -> Status {
        if (!*inited) { *inited = true; p.set_trap_exit(true); srv->init(p); }
        auto m = p.receive();
        if (!m) return Status::Wait;
        const Term& t = m->body;
        const Term* tag = tuple_at(t, 0);
        if (tag && is_atom(*tag, "$gen_call") && tuple_size(t) == 3) {
            const Term* from = tuple_at(t, 1);
            const Pid* caller = from ? tuple_at(*from, 0)->get_if<Pid>() : nullptr;
            const Ref* ref = from ? tuple_at(*from, 1)->get_if<Ref>() : nullptr;
            CallResult r = srv->handle_call(p, *tuple_at(t, 2), caller ? *caller : Pid{});
            if (caller && ref) p.send(*caller, tuple({Term(*ref), r.reply}));
            if (r.stop) { srv->terminate(p, *r.stop); return p.stop(*r.stop); }
        } else if (tag && is_atom(*tag, "$gen_cast") && tuple_size(t) == 2) {
            srv->handle_cast(p, *tuple_at(t, 1));
        } else if (tag && is_atom(*tag, "EXIT") && tuple_size(t) == 3) {
            const Pid* from = tuple_at(t, 1)->get_if<Pid>();
            if (from && *from == p.parent()) {
                srv->terminate(p, *tuple_at(t, 2));
                return p.stop(*tuple_at(t, 2));
            }
            srv->handle_info(p, t);
        } else {
            srv->handle_info(p, t);
        }
        return Status::Continue;
    };
}

inline Pid start_gen_server(Runtime& rt, std::shared_ptr<GenServer> srv, SpawnOptions opts = {}) {
    return rt.spawn(gen_server_body(std::move(srv)), std::move(opts));
}
inline Pid start_gen_server_link(Process& parent, std::shared_ptr<GenServer> srv, SpawnOptions opts = {}) {
    return parent.spawn_link(gen_server_body(std::move(srv)), std::move(opts));
}

// Sisi klien (non-blocking): kirim call, lalu ambil balasan di slice berikutnya.
inline Ref gen_call(Process& caller, Pid server, Term request) {
    Ref r = caller.runtime().make_ref();
    caller.send(server, tuple({atom("$gen_call"), tuple({Term(caller.self()), Term(r)}), std::move(request)}));
    return r;
}
inline std::optional<Term> gen_take_reply(Process& caller, Ref ref) {
    auto m = caller.receive([&](const Message& msg) {
        const Term* r = tuple_at(msg.body, 0);
        return tuple_size(msg.body) == 2 && r && r->get_if<Ref>() && *r->get_if<Ref>() == ref;
    });
    if (!m) return std::nullopt;
    return *tuple_at(m->body, 1);
}
inline void gen_cast(Process& from, Pid server, Term msg) {
    from.send(server, tuple({atom("$gen_cast"), std::move(msg)}));
}
inline void gen_cast(Runtime& rt, Pid server, Term msg) {
    rt.send(Pid{}, server, tuple({atom("$gen_cast"), std::move(msg)}));
}

}  // namespace garuda::beam
