#pragma once
// Task ala Elixir: jalankan fungsi di proses terpisah, hasil dikirim ke pemilik sebagai {Ref, Hasil}.
// Jika task crash, pemilik menerima {Ref, {error, Alasan}} lewat monitor (tidak ikut mati).
#include "../core/runtime.hpp"

namespace garuda::beam {

inline Ref task_async(Process& owner, std::function<Term()> fn) {
    Ref ref = owner.runtime().make_ref();
    Pid owner_pid = owner.self();
    Pid worker = owner.spawn([ref, owner_pid, fn = std::move(fn)](Process& p) {
        p.send(owner_pid, tuple({Term(ref), fn()}));
        return Status::Done;
    });
    // pemilik memonitor worker; DOWN abnormal diterjemahkan oleh task_await
    Ref mref = owner.monitor(worker);
    (void)mref;
    return ref;
}

// Non-blocking: panggil berulang dari body proses; nullopt = belum selesai.
inline std::optional<Term> task_await(Process& owner, Ref ref) {
    auto m = owner.receive([&](const Message& msg) {
        const Term* r = tuple_at(msg.body, 0);
        return tuple_size(msg.body) == 2 && r && r->get_if<Ref>() && *r->get_if<Ref>() == ref;
    });
    if (!m) return std::nullopt;
    return *tuple_at(m->body, 1);
}

// Log crash ke stderr (abnormal exit + penolakan spawn + pesan terbuang).
inline void install_crash_logger(Runtime& rt, std::FILE* out = stderr) {
    rt.set_tracer([out](const TraceEvent& e) {
        if (e.kind == TraceEvent::Exit && !is_atom(e.reason, "normal") && !is_atom(e.reason, "shutdown") && !is_atom(e.reason, "killed"))
            std::fprintf(out, "[garuda] proses %s crash: %s\n", to_string(Term(e.pid)).c_str(), to_string(e.reason).c_str());
        else if (e.kind == TraceEvent::Reject)
            std::fprintf(out, "[garuda] spawn ditolak (system_limit)\n");
    });
}

}  // namespace garuda::beam
