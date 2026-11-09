#pragma once
// Garuda FFI: Python Language Interoperability (`python_ffi.hpp`)
// Mengimplementasikan Python CAPI / PyObject Binding Simulation, Module Evaluator, & Term Conversion.

#include "../core/term.hpp"
#include "c_abi.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace garuda::beam {

struct PyObjectMock {
    std::string type;
    std::string str_val;
    int64_t int_val = 0;
    double float_val = 0.0;
    bool bool_val = false;
    std::vector<PyObjectMock> list_val;
    std::map<std::string, PyObjectMock> dict_val;

    std::string to_string() const {
        if (type == "int") return std::to_string(int_val);
        if (type == "float") return std::to_string(float_val);
        if (type == "str") return "\"" + str_val + "\"";
        if (type == "bool") return bool_val ? "True" : "False";
        return "None";
    }
};

class PythonFfiBridge {
public:
    PythonFfiBridge() : initialized_(true) {}

    bool is_initialized() const { return initialized_; }

    PyObjectMock eval_script(const std::string& script) {
        std::lock_guard<std::mutex> lock(mu_);
        PyObjectMock res;
        if (script.find("def ") != std::string::npos) {
            res.type = "str";
            res.str_val = "function_defined";
        } else if (script.find("print") != std::string::npos) {
            res.type = "str";
            res.str_val = "printed";
        } else if (script.find("+") != std::string::npos || script.find("*") != std::string::npos) {
            res.type = "int";
            res.int_val = 42;
        } else {
            res.type = "str";
            res.str_val = script;
        }
        return res;
    }

    PyObjectMock call_function(const std::string& func_name, const std::vector<PyObjectMock>& args) {
        std::lock_guard<std::mutex> lock(mu_);
        PyObjectMock res;
        res.type = "str";
        res.str_val = "py_result_for_" + func_name;
        if (!args.empty() && args[0].type == "int") {
            res.type = "int";
            res.int_val = args[0].int_val * 2;
        }
        return res;
    }

    Term to_garuda_term(const PyObjectMock& py_obj) {
        if (py_obj.type == "int") return Term(py_obj.int_val);
        if (py_obj.type == "float") return Term(py_obj.float_val);
        if (py_obj.type == "str") return Term(py_obj.str_val);
        if (py_obj.type == "bool") return Term(py_obj.bool_val);
        return Term(Atom{"null"});
    }

    PyObjectMock from_garuda_term(const Term& term) {
        PyObjectMock py_obj;
        if (auto* i = term.get_if<int64_t>()) {
            py_obj.type = "int";
            py_obj.int_val = *i;
        } else if (auto* f = term.get_if<double>()) {
            py_obj.type = "float";
            py_obj.float_val = *f;
        } else if (auto* s = term.get_if<std::string>()) {
            py_obj.type = "str";
            py_obj.str_val = *s;
        } else if (auto* a = term.get_if<Atom>()) {
            py_obj.type = "bool";
            py_obj.bool_val = (a->name == "true" || a->name == "benar");
        } else {
            py_obj.type = "none";
        }
        return py_obj;
    }

private:
    bool initialized_;
    std::mutex mu_;
};

} // namespace garuda::beam
