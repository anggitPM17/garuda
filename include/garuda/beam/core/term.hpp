#pragma once
// Garuda BEAM-core: tipe data Term (mirip term Erlang) dengan urutan total.
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace garuda::beam {

struct Pid {
    std::uint64_t id = 0;
    bool valid() const { return id != 0; }
    friend bool operator==(Pid a, Pid b) { return a.id == b.id; }
    friend bool operator!=(Pid a, Pid b) { return a.id != b.id; }
    friend bool operator<(Pid a, Pid b) { return a.id < b.id; }
};

struct Ref {
    std::uint64_t id = 0;
    friend bool operator==(Ref a, Ref b) { return a.id == b.id; }
    friend bool operator!=(Ref a, Ref b) { return a.id != b.id; }
    friend bool operator<(Ref a, Ref b) { return a.id < b.id; }
};

struct Atom {
    std::string name;
};

class Term;
using Seq = std::shared_ptr<const std::vector<Term>>;  // immutable & murah di-copy (seperti BEAM)
struct Tuple { Seq items; };
struct List  { Seq items; };

class Term {
public:
    using Value = std::variant<std::monostate, std::int64_t, double, Atom, std::string,
                               Pid, Ref, Tuple, List>;

    Term() = default;
    template <class I, class = std::enable_if_t<std::is_integral_v<I> && !std::is_same_v<I, bool>>>
    Term(I v) : v_(static_cast<std::int64_t>(v)) {}
    Term(bool b) : v_(Atom{b ? "true" : "false"}) {}
    Term(double d) : v_(d) {}
    Term(const char* s) : v_(std::string(s)) {}
    Term(std::string s) : v_(std::move(s)) {}
    Term(Atom a) : v_(std::move(a)) {}
    Term(Pid p) : v_(p) {}
    Term(Ref r) : v_(r) {}
    Term(Tuple t) : v_(std::move(t)) {}
    Term(List l) : v_(std::move(l)) {}

    const Value& value() const { return v_; }
    template <class T> const T* get_if() const { return std::get_if<T>(&v_); }
    template <class T> bool is() const { return std::holds_alternative<T>(v_); }

private:
    Value v_;
};

inline const std::vector<Term>& seq_items(const Seq& s) {
    static const std::vector<Term> empty;
    return s ? *s : empty;
}

inline Term atom(std::string name) { return Term(Atom{std::move(name)}); }
inline Term tuple_of(std::vector<Term> items) {
    return Term(Tuple{std::make_shared<const std::vector<Term>>(std::move(items))});
}
inline Term tuple(std::initializer_list<Term> items) { return tuple_of(std::vector<Term>(items)); }
inline Term list_of(std::vector<Term> items) {
    return Term(List{std::make_shared<const std::vector<Term>>(std::move(items))});
}
inline Term list(std::initializer_list<Term> items) { return list_of(std::vector<Term>(items)); }

inline bool is_atom(const Term& t, std::string_view name) {
    const Atom* a = t.get_if<Atom>();
    return a && a->name == name;
}
inline std::size_t tuple_size(const Term& t) {
    const Tuple* tp = t.get_if<Tuple>();
    return tp ? seq_items(tp->items).size() : 0;
}
inline const Term* tuple_at(const Term& t, std::size_t i) {
    const Tuple* tp = t.get_if<Tuple>();
    if (!tp) return nullptr;
    const auto& v = seq_items(tp->items);
    return i < v.size() ? &v[i] : nullptr;
}

// ---- urutan total antar term: number < atom < ref < pid < tuple < list < string ----
inline int term_rank(const Term& t) {
    switch (t.value().index()) {
        case 0: return 0;
        case 1: case 2: return 1;
        case 3: return 2;
        case 6: return 3;
        case 5: return 4;
        case 7: return 5;
        case 8: return 6;
        default: return 7;
    }
}
template <class T> inline int cmp3(const T& a, const T& b) { return a < b ? -1 : (b < a ? 1 : 0); }

inline int compare(const Term& a, const Term& b) {
    int ra = term_rank(a), rb = term_rank(b);
    if (ra != rb) return cmp3(ra, rb);
    switch (ra) {
        case 0: return 0;
        case 1: {
            const auto* ia = a.get_if<std::int64_t>();
            const auto* ib = b.get_if<std::int64_t>();
            if (ia && ib) return cmp3(*ia, *ib);
            double da = ia ? static_cast<double>(*ia) : *a.get_if<double>();
            double db = ib ? static_cast<double>(*ib) : *b.get_if<double>();
            return cmp3(da, db);
        }
        case 2: return cmp3(a.get_if<Atom>()->name, b.get_if<Atom>()->name);
        case 3: return cmp3(a.get_if<Ref>()->id, b.get_if<Ref>()->id);
        case 4: return cmp3(a.get_if<Pid>()->id, b.get_if<Pid>()->id);
        case 5: case 6: {
            const auto& va = ra == 5 ? seq_items(a.get_if<Tuple>()->items) : seq_items(a.get_if<List>()->items);
            const auto& vb = ra == 5 ? seq_items(b.get_if<Tuple>()->items) : seq_items(b.get_if<List>()->items);
            if (ra == 5 && va.size() != vb.size()) return cmp3(va.size(), vb.size());
            std::size_t n = va.size() < vb.size() ? va.size() : vb.size();
            for (std::size_t i = 0; i < n; ++i)
                if (int c = compare(va[i], vb[i])) return c;
            return cmp3(va.size(), vb.size());
        }
        default: return cmp3(*a.get_if<std::string>(), *b.get_if<std::string>());
    }
}
inline bool operator==(const Term& a, const Term& b) { return compare(a, b) == 0; }
inline bool operator!=(const Term& a, const Term& b) { return compare(a, b) != 0; }
inline bool operator<(const Term& a, const Term& b) { return compare(a, b) < 0; }
struct TermLess { bool operator()(const Term& a, const Term& b) const { return compare(a, b) < 0; } };

inline std::string to_string(const Term& t) {
    std::ostringstream os;
    switch (term_rank(t)) {
        case 0: os << "undefined"; break;
        case 1:
            if (auto* i = t.get_if<std::int64_t>()) os << *i; else os << *t.get_if<double>();
            break;
        case 2: os << t.get_if<Atom>()->name; break;
        case 3: os << "#Ref<" << t.get_if<Ref>()->id << ">"; break;
        case 4: os << "<0." << t.get_if<Pid>()->id << ".0>"; break;
        case 5: case 6: {
            bool is_t = term_rank(t) == 5;
            const auto& v = is_t ? seq_items(t.get_if<Tuple>()->items) : seq_items(t.get_if<List>()->items);
            os << (is_t ? '{' : '[');
            for (std::size_t i = 0; i < v.size(); ++i) os << (i ? "," : "") << to_string(v[i]);
            os << (is_t ? '}' : ']');
            break;
        }
        default: os << '"' << *t.get_if<std::string>() << '"';
    }
    return os.str();
}

}  // namespace garuda::beam
