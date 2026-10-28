#pragma once
// =====================================================================
// FASE G1.5 - GARUDA GEN_STATEM (State Machine Behaviour)
// Setara Erlang/OTP gen_statem.erl — dengan sintaks C++ sederhana.
// Callback: init/3, handle_event/4, terminate/4
// =====================================================================
#include "../core/runtime.hpp"

#include <memory>
#include <string>
#include <optional>

namespace garuda::beam {

struct StatemNextState {
    Term state;
    Term data;
    std::optional<Term> reply;
    std::optional<Term> stop;
};

class GenStatem {
public:
    virtual ~GenStatem() = default;

    virtual void init(Process&, Term& state, Term& data) {
        state = atom("idle");
        data = atom("none");
    }

    virtual StatemNextState handle_event(Process&, const Term& event, const Term& state, const Term& data) = 0;

    virtual void terminate(Process&, const Term& reason, const Term& state, const Term& data) {
        (void)reason; (void)state; (void)data;
    }
};

namespace detail {

struct StatemContext {
    std::shared_ptr<GenStatem> impl;
    Term state;
    Term data;
    bool inited = false;
};

inline Body gen_statem_body(std::shared_ptr<GenStatem> impl, Term initial_state, Term initial_data) {
    auto ctx = std::make_shared<StatemContext>();
    ctx->impl = std::move(impl);
    ctx->state = std::move(initial_state);
    ctx->data = std::move(initial_data);

    return [ctx](Process& p) -> Status {
        if (!ctx->inited) {
            ctx->inited = true;
            p.set_trap_exit(true);
            ctx->impl->init(p, ctx->state, ctx->data);
        }

        auto m = p.receive();
        if (!m) return Status::Wait;

        const Term& t = m->body;

        if (tuple_size(t) >= 2 && is_atom(*tuple_at(t, 0), "$statem_event")) {
            const Term& event = *tuple_at(t, 1);
            try {
                StatemNextState next = ctx->impl->handle_event(p, event, ctx->state, ctx->data);
                ctx->state = std::move(next.state);
                ctx->data = std::move(next.data);
                if (next.stop) {
                    ctx->impl->terminate(p, *next.stop, ctx->state, ctx->data);
                    return p.stop(*next.stop);
                }
            } catch (const std::exception& e) {
                ctx->impl->terminate(p, atom("handle_event_exception"), ctx->state, ctx->data);
                return p.stop(Term(std::string(e.what())));
            } catch (...) {
                ctx->impl->terminate(p, atom("handle_event_unknown_exception"), ctx->state, ctx->data);
                return p.stop(atom("crash"));
            }
        } else if (tuple_size(t) == 4 && is_atom(*tuple_at(t, 0), "$statem_call")) {
            const Pid* caller = tuple_at(t, 1)->get_if<Pid>();
            const Ref* ref = tuple_at(t, 2)->get_if<Ref>();
            const Term& event = *tuple_at(t, 3);
            try {
                StatemNextState next = ctx->impl->handle_event(p, event, ctx->state, ctx->data);
                ctx->state = std::move(next.state);
                ctx->data = std::move(next.data);

                if (caller && ref) {
                    Term reply = next.reply.value_or(atom("ok"));
                    p.send(*caller, tuple({Term(*ref), reply}));
                }

                if (next.stop) {
                    ctx->impl->terminate(p, *next.stop, ctx->state, ctx->data);
                    return p.stop(*next.stop);
                }
            } catch (const std::exception& e) {
                if (caller && ref) {
                    p.send(*caller, tuple({Term(*ref), tuple({atom("error"), Term(std::string(e.what()))})}));
                }
                ctx->impl->terminate(p, atom("handle_event_exception"), ctx->state, ctx->data);
                return p.stop(Term(std::string(e.what())));
            }
        } else if (tuple_size(t) == 3 && is_atom(*tuple_at(t, 0), "EXIT")) {
            const Pid* from = tuple_at(t, 1)->get_if<Pid>();
            const Term& reason = *tuple_at(t, 2);
            if (from && *from == p.parent()) {
                ctx->impl->terminate(p, reason, ctx->state, ctx->data);
                return p.stop(reason);
            }
        }

        return Status::Continue;
    };
}

} // namespace detail

inline Pid start_gen_statem(Runtime& rt,
                            std::shared_ptr<GenStatem> impl,
                            Term initial_state = atom("idle"),
                            Term initial_data = atom("none"),
                            SpawnOptions opts = {}) {
    return rt.spawn(detail::gen_statem_body(std::move(impl), std::move(initial_state), std::move(initial_data)),
                    std::move(opts));
}

inline Pid start_gen_statem_link(Process& parent,
                                  std::shared_ptr<GenStatem> impl,
                                  Term initial_state = atom("idle"),
                                  Term initial_data = atom("none"),
                                  SpawnOptions opts = {}) {
    opts.link = true;
    opts.parent = parent.self();
    return parent.spawn_link(detail::gen_statem_body(std::move(impl), std::move(initial_state), std::move(initial_data)),
                             std::move(opts));
}

inline void gen_statem_cast(Process& from, Pid server, Term event) {
    from.send(server, tuple({atom("$statem_event"), std::move(event)}));
}

inline void gen_statem_cast(Runtime& rt, Pid server, Term event) {
    rt.send(Pid{}, server, tuple({atom("$statem_event"), std::move(event)}));
}

inline Ref gen_statem_call(Process& caller, Pid server, Term event) {
    Ref r = caller.runtime().make_ref();
    caller.send(server, tuple({atom("$statem_call"), Term(caller.self()), Term(r), std::move(event)}));
    return r;
}

inline std::optional<Term> gen_statem_take_reply(Process& caller, Ref ref) {
    auto m = caller.receive([&](const Message& msg) {
        const Term* r = tuple_at(msg.body, 0);
        return tuple_size(msg.body) == 2 && r && r->get_if<Ref>() && *r->get_if<Ref>() == ref;
    });
    if (!m) return std::nullopt;
    return *tuple_at(m->body, 1);
}

} // namespace garuda::beam
