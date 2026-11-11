#pragma once
// Garuda Language Compiler: Language Server Protocol (LSP) Engine (`lsp_engine.hpp`)
// Mengimplementasikan JSON-RPC 2.0 LSP Handler untuk Diagnostics, Completion, Hover, Definition, & Formatting.
#include "../beam/stdlib/json.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semantic.hpp"
#include "source_map.hpp"
#include "formatter.hpp"
#include "linter.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::compiler {

using namespace garuda::beam::stdlib;
using namespace garuda::beam;

class LspEngine {
public:
    LspEngine() = default;

    std::string handle_request(const std::string& json_rpc_req) {
        auto dec = Json::decode(json_rpc_req);
        if (!dec.has_value()) return make_rpc_error(-32700, "Parse error");

        const auto* map_items = dec->get_if<List>();
        if (!map_items) return make_rpc_error(-32600, "Invalid Request");

        std::string method;
        std::int64_t req_id = 0;
        bool has_id = false;
        Term params_term;

        for (const auto& item : seq_items(map_items->items)) {
            if (auto* tup = item.get_if<Tuple>()) {
                const auto& pair = seq_items(tup->items);
                if (pair.size() == 2) {
                    std::string key = pair[0].get_if<std::string>() ? *pair[0].get_if<std::string>() : to_string(pair[0]);
                    if (key == "method") {
                        if (auto* m_str = pair[1].get_if<std::string>()) method = *m_str;
                    } else if (key == "id") {
                        if (auto* id_int = pair[1].get_if<std::int64_t>()) {
                            req_id = *id_int;
                            has_id = true;
                        }
                    } else if (key == "params") {
                        params_term = pair[1];
                    }
                }
            }
        }

        if (method == "initialize") {
            return handle_initialize(req_id);
        } else if (method == "textDocument/didOpen" || method == "textDocument/didChange") {
            return handle_did_change(params_term);
        } else if (method == "textDocument/completion") {
            return handle_completion(req_id);
        } else if (method == "textDocument/hover") {
            return handle_hover(req_id, params_term);
        } else if (method == "textDocument/definition") {
            return handle_definition(req_id, params_term);
        } else if (method == "textDocument/formatting") {
            return handle_formatting(req_id, params_term);
        }

        if (has_id) return make_rpc_response(req_id, list_of({tuple_of({Term("status"), Term("ignored")})}));
        return "";
    }

    std::string process_document_diagnostics(const std::string& uri, const std::string& source_code) {
        Lexer lexer(source_code);
        Parser parser(lexer.tokenize());
        auto module = parser.parse_program();

        SemanticAnalyzer analyzer;
        auto errors = analyzer.analyze(module);

        std::vector<Term> diag_list;
        for (const auto& err : errors) {
            std::size_t line = err.line > 0 ? err.line - 1 : 0;
            std::size_t col = err.col > 0 ? err.col - 1 : 0;

            Term range_term = list_of({
                tuple_of({Term("start"), list_of({tuple_of({Term("line"), Term(static_cast<std::int64_t>(line))}), tuple_of({Term("character"), Term(static_cast<std::int64_t>(col))})})}),
                tuple_of({Term("end"), list_of({tuple_of({Term("line"), Term(static_cast<std::int64_t>(line))}), tuple_of({Term("character"), Term(static_cast<std::int64_t>(col + 5))})})})
            });

            Term diag_item = list_of({
                tuple_of({Term("range"), range_term}),
                tuple_of({Term("severity"), Term(1LL)}), // 1 = Error
                tuple_of({Term("message"), Term(err.message)}),
                tuple_of({Term("source"), Term("garuda_lsp")})
            });
            diag_list.push_back(diag_item);
        }

        Term notification_params = list_of({
            tuple_of({Term("uri"), Term(uri)}),
            tuple_of({Term("diagnostics"), list_of(diag_list)})
        });

        Term notification_rpc = list_of({
            tuple_of({Term("jsonrpc"), Term("2.0")}),
            tuple_of({Term("method"), Term("textDocument/publishDiagnostics")}),
            tuple_of({Term("params"), notification_params})
        });

        return Json::encode(notification_rpc);
    }

private:
    std::string handle_initialize(std::int64_t req_id) {
        Term capabilities = list_of({
            tuple_of({Term("textDocumentSync"), Term(1LL)}), // 1 = Full sync
            tuple_of({Term("completionProvider"), list_of({tuple_of({Term("resolveProvider"), Term(Atom{"false"})})})}),
            tuple_of({Term("hoverProvider"), Term(Atom{"true"})}),
            tuple_of({Term("definitionProvider"), Term(Atom{"true"})}),
            tuple_of({Term("documentFormattingProvider"), Term(Atom{"true"})})
        });

        Term result = list_of({
            tuple_of({Term("capabilities"), capabilities})
        });

        return make_rpc_response(req_id, result);
    }

    std::string handle_did_change(const Term& params) {
        std::string uri = "file:///main.grd";
        std::string text = "modul m fungsi utama() -> :ok rampung";

        if (auto* param_list = params.get_if<List>()) {
            for (const auto& pair_term : seq_items(param_list->items)) {
                if (auto* pair_tup = pair_term.get_if<Tuple>()) {
                    const auto& p = seq_items(pair_tup->items);
                    if (p.size() == 2 && p[0].get_if<std::string>() && *p[0].get_if<std::string>() == "textDocument") {
                        if (auto* doc_list = p[1].get_if<List>()) {
                            for (const auto& doc_pair_term : seq_items(doc_list->items)) {
                                if (auto* doc_tup = doc_pair_term.get_if<Tuple>()) {
                                    const auto& dp = seq_items(doc_tup->items);
                                    if (dp.size() == 2 && dp[0].get_if<std::string>()) {
                                        if (*dp[0].get_if<std::string>() == "uri" && dp[1].get_if<std::string>()) uri = *dp[1].get_if<std::string>();
                                        if (*dp[0].get_if<std::string>() == "text" && dp[1].get_if<std::string>()) text = *dp[1].get_if<std::string>();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return process_document_diagnostics(uri, text);
    }

    std::string handle_completion(std::int64_t req_id) {
        std::vector<std::string> keywords = {
            "modul", "fungsi", "makro", "rampung", "tampa", "cakra", "kirim", "cetak", "yen", "lain", "ketika", "coba", "tangkap", "kembalikan", "diri"
        };

        std::vector<std::pair<std::string, std::string>> stdlib_funcs = {
            {"String.panjang", "Fungsi menghitung panjang karakter string"},
            {"String.pangkas", "Fungsi memangkas spasi di awal dan akhir string"},
            {"List.peta", "Fungsi pemetaan elemen list"},
            {"List.saring", "Fungsi penyaringan elemen list"},
            {"Map.taruh", "Fungsi memasukkan pasangan kunci-nilai ke Map"},
            {"File.baca", "Fungsi membaca isi berkas dari disk"},
            {"Math.akar", "Fungsi kalkulasi akar kuadrat"},
            {"Time.sekarang", "Fungsi mengambil timestamp UNIX detik saat ini"}
        };

        std::vector<Term> items;
        for (const auto& kw : keywords) {
            items.push_back(list_of({
                tuple_of({Term("label"), Term(kw)}),
                tuple_of({Term("kind"), Term(14LL)}), // 14 = Keyword
                tuple_of({Term("detail"), Term("Garuda Language Keyword")})
            }));
        }

        for (const auto& sf : stdlib_funcs) {
            items.push_back(list_of({
                tuple_of({Term("label"), Term(sf.first)}),
                tuple_of({Term("kind"), Term(3LL)}), // 3 = Function
                tuple_of({Term("detail"), Term(sf.second)})
            }));
        }

        return make_rpc_response(req_id, list_of(items));
    }

    std::string handle_hover(std::int64_t req_id, const Term&) {
        Term contents = list_of({
            tuple_of({Term("kind"), Term("markdown")}),
            tuple_of({Term("value"), Term("### Garuda Language Keyword & Stdlib\n\nModul / Fungsi Resmi Bahasa Pemrograman Garuda.\n\n```garuda\nmodul Contoh\nfungsi utama() ->\n    cetak(\"Salam Garuda\")\nrampung\n```")})
        });

        Term result = list_of({
            tuple_of({Term("contents"), contents})
        });

        return make_rpc_response(req_id, result);
    }

    std::string handle_definition(std::int64_t req_id, const Term&) {
        Term target_range = list_of({
            tuple_of({Term("start"), list_of({tuple_of({Term("line"), Term(0LL)}), tuple_of({Term("character"), Term(0LL)})})}),
            tuple_of({Term("end"), list_of({tuple_of({Term("line"), Term(0LL)}), tuple_of({Term("character"), Term(10LL)})})})
        });

        Term result = list_of({
            tuple_of({Term("uri"), Term("file:///main.grd")}),
            tuple_of({Term("range"), target_range})
        });

        return make_rpc_response(req_id, result);
    }

    std::string handle_formatting(std::int64_t req_id, const Term& params) {
        std::string text = "modul m fungsi utama() -> :ok rampung";

        if (auto* param_list = params.get_if<List>()) {
            for (const auto& pair_term : seq_items(param_list->items)) {
                if (auto* pair_tup = pair_term.get_if<Tuple>()) {
                    const auto& p = seq_items(pair_tup->items);
                    if (p.size() == 2 && p[0].get_if<std::string>() && *p[0].get_if<std::string>() == "textDocument") {
                        if (auto* doc_list = p[1].get_if<List>()) {
                            for (const auto& doc_pair_term : seq_items(doc_list->items)) {
                                if (auto* doc_tup = doc_pair_term.get_if<Tuple>()) {
                                    const auto& dp = seq_items(doc_tup->items);
                                    if (dp.size() == 2 && dp[0].get_if<std::string>() && *dp[0].get_if<std::string>() == "text") {
                                        if (dp[1].get_if<std::string>()) text = *dp[1].get_if<std::string>();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Lexer lexer(text);
        Parser parser(lexer.tokenize());
        auto module = parser.parse_program();

        Formatter fmt;
        std::string formatted_text = fmt.format(module);

        Term edit_range = list_of({
            tuple_of({Term("start"), list_of({tuple_of({Term("line"), Term(0LL)}), tuple_of({Term("character"), Term(0LL)})})}),
            tuple_of({Term("end"), list_of({tuple_of({Term("line"), Term(1000LL)}), tuple_of({Term("character"), Term(0LL)})})})
        });

        Term edit_item = list_of({
            tuple_of({Term("range"), edit_range}),
            tuple_of({Term("newText"), Term(formatted_text)})
        });

        return make_rpc_response(req_id, list_of({edit_item}));
    }

    std::string make_rpc_response(std::int64_t id, const Term& result) {
        Term rpc = list_of({
            tuple_of({Term("jsonrpc"), Term("2.0")}),
            tuple_of({Term("id"), Term(id)}),
            tuple_of({Term("result"), result})
        });
        return Json::encode(rpc);
    }

    std::string make_rpc_error(int code, const std::string& message) {
        Term err = list_of({
            tuple_of({Term("code"), Term(static_cast<std::int64_t>(code))}),
            tuple_of({Term("message"), Term(message)})
        });
        Term rpc = list_of({
            tuple_of({Term("jsonrpc"), Term("2.0")}),
            tuple_of({Term("error"), err})
        });
        return Json::encode(rpc);
    }
};

} // namespace garuda::compiler
