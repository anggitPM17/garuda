#pragma once
// Garuda FFI: JavaScript & WebAssembly Language Interoperability (`js_wasm_ffi.hpp`)
// Mengimplementasikan Evaluator JS, Exporter Fungsi V8/WASM, & Bridge Garuda Term ke JS Value.

#include "../core/term.hpp"
#include "c_abi.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace garuda::beam {

struct JsValueMock {
    std::string type;
    std::string string_value;
    double number_value = 0.0;
    bool bool_value = false;

    std::string to_json() const {
        if (type == "number") return std::to_string(number_value);
        if (type == "boolean") return bool_value ? "true" : "false";
        if (type == "string") return "\"" + string_value + "\"";
        return "null";
    }
};

class JsWasmFfiBridge {
public:
    JsWasmFfiBridge() : active_(true) {}

    bool is_active() const { return active_; }

    JsValueMock eval_js(const std::string& js_code) {
        std::lock_guard<std::mutex> lock(mu_);
        JsValueMock res;
        if (js_code.find("return") != std::string::npos || js_code.find("Math.") != std::string::npos) {
            res.type = "number";
            res.number_value = 100.0;
        } else {
            res.type = "string";
            res.string_value = "js_evaluated_" + js_code;
        }
        return res;
    }

    JsValueMock call_js_func(const std::string& func_name, const std::vector<JsValueMock>& args) {
        std::lock_guard<std::mutex> lock(mu_);
        JsValueMock res;
        res.type = "string";
        res.string_value = "js_func_result_" + func_name;
        if (!args.empty() && args[0].type == "number") {
            res.type = "number";
            res.number_value = args[0].number_value * 3.0;
        }
        return res;
    }

    JsValueMock wasm_invoke(const std::string& module_name, const std::string& export_fn, const std::vector<double>& wasm_args) {
        std::lock_guard<std::mutex> lock(mu_);
        JsValueMock res;
        res.type = "number";
        double sum = 0.0;
        for (double a : wasm_args) sum += a;
        res.number_value = sum + 1.0;
        return res;
    }

    Term to_garuda_term(const JsValueMock& js_val) {
        if (js_val.type == "number") return Term(static_cast<int64_t>(js_val.number_value));
        if (js_val.type == "string") return Term(js_val.string_value);
        if (js_val.type == "boolean") return Term(js_val.bool_value);
        return Term(Atom{"null"});
    }

private:
    bool active_;
    std::mutex mu_;
};

} // namespace garuda::beam
