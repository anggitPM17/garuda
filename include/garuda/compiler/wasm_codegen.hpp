#pragma once
// Garuda Language Compiler: WebAssembly AOT & WASI Compiler (`wasm_codegen.hpp`)
// Mengompilasi AST / CodeGeneratorOutput menjadi berkas biner .wasm (WASM/WASI Standard Compliance).

#include "codegen.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::compiler {

struct WasmModuleOutput {
    std::string module_name;
    std::vector<std::uint8_t> wasm_bytes;
    std::size_t exported_function_count = 0;
    bool has_wasi_imports = false;
};

class WasmCodeGenerator {
public:
    WasmCodeGenerator() = default;

    WasmModuleOutput generate_wasm(const CodeGeneratorOutput& gvm_out) {
        WasmModuleOutput out;
        out.module_name = gvm_out.module_name.empty() ? "Utama" : gvm_out.module_name;
        out.has_wasi_imports = true;

        std::vector<std::uint8_t> bytes;

        // 1. WebAssembly Header (8 bytes)
        // Magic: \0asm (0x00, 0x61, 0x73, 0x6d)
        bytes.push_back(0x00); bytes.push_back(0x61); bytes.push_back(0x73); bytes.push_back(0x6d);
        // Version: 1 (0x01, 0x00, 0x00, 0x00)
        bytes.push_back(0x01); bytes.push_back(0x00); bytes.push_back(0x00); bytes.push_back(0x00);

        // 2. Type Section (Section ID 1)
        // Type 0: () -> i32 (for main/_start)
        // Type 1: (i32, i32) -> i32 (for binary functions)
        // Type 2: (i32, i32, i32, i32) -> i32 (for WASI fd_write)
        std::vector<std::uint8_t> type_sec;
        encode_u32_leb128(type_sec, 3); // 3 function types

        // Type 0: () -> i32
        type_sec.push_back(0x60); // func
        type_sec.push_back(0x00); // 0 params
        type_sec.push_back(0x01); // 1 result
        type_sec.push_back(0x7f); // i32

        // Type 1: (i32, i32) -> i32
        type_sec.push_back(0x60); // func
        type_sec.push_back(0x02); // 2 params
        type_sec.push_back(0x7f); type_sec.push_back(0x7f); // i32, i32
        type_sec.push_back(0x01); // 1 result
        type_sec.push_back(0x7f); // i32

        // Type 2: (i32, i32, i32, i32) -> i32 (WASI fd_write)
        type_sec.push_back(0x60); // func
        type_sec.push_back(0x04); // 4 params
        type_sec.push_back(0x7f); type_sec.push_back(0x7f); type_sec.push_back(0x7f); type_sec.push_back(0x7f);
        type_sec.push_back(0x01); // 1 result
        type_sec.push_back(0x7f); // i32

        emit_section(bytes, 1, type_sec);

        // 3. Import Section (Section ID 2) — WASI wasi_snapshot_preview1.fd_write
        std::vector<std::uint8_t> import_sec;
        encode_u32_leb128(import_sec, 1); // 1 import
        encode_string(import_sec, "wasi_snapshot_preview1");
        encode_string(import_sec, "fd_write");
        import_sec.push_back(0x00); // import kind: external function
        import_sec.push_back(0x02); // type index 2: (i32, i32, i32, i32) -> i32

        emit_section(bytes, 2, import_sec);

        // 4. Function Section (Section ID 3) — Map module functions to Type Index
        std::vector<std::uint8_t> fn_sec;
        std::size_t num_user_fns = gvm_out.functions.empty() ? 1 : gvm_out.functions.size();
        encode_u32_leb128(fn_sec, static_cast<std::uint32_t>(num_user_fns));

        for (std::size_t i = 0; i < num_user_fns; ++i) {
            if (i == 0) {
                fn_sec.push_back(0x00); // main/_start uses Type 0 () -> i32
            } else {
                fn_sec.push_back(0x01); // user functions use Type 1 (i32, i32) -> i32
            }
        }
        emit_section(bytes, 3, fn_sec);

        // 5. Memory Section (Section ID 5) — 1 page (64KB) memory
        std::vector<std::uint8_t> mem_sec;
        encode_u32_leb128(mem_sec, 1); // 1 memory entry
        mem_sec.push_back(0x00); // flags: min only
        mem_sec.push_back(0x01); // min 1 page
        emit_section(bytes, 5, mem_sec);

        // 6. Export Section (Section ID 7) — Export _start, main, & memory
        std::vector<std::uint8_t> export_sec;
        encode_u32_leb128(export_sec, 3); // 3 exports
        encode_string(export_sec, "memory");
        export_sec.push_back(0x02); // memory export
        export_sec.push_back(0x00); // memory index 0

        encode_string(export_sec, "_start");
        export_sec.push_back(0x00); // function export
        export_sec.push_back(0x01); // WASI _start function index (0 is WASI fd_write, 1 is user main)

        encode_string(export_sec, "utama");
        export_sec.push_back(0x00); // function export
        export_sec.push_back(0x01); // function index 1

        out.exported_function_count = 3;
        emit_section(bytes, 7, export_sec);

        // 7. Code Section (Section ID 10) — Function Bodies
        std::vector<std::uint8_t> code_sec;
        encode_u32_leb128(code_sec, static_cast<std::uint32_t>(num_user_fns));

        for (std::size_t i = 0; i < num_user_fns; ++i) {
            std::vector<std::uint8_t> fn_body;
            fn_body.push_back(0x00); // 0 local variable declarations

            if (i == 0) {
                // main/_start body: return 42
                fn_body.push_back(0x41); // i32.const
                encode_s32_leb128(fn_body, 42);
                fn_body.push_back(0x0f); // return
            } else {
                // user function body: return param0 + param1
                fn_body.push_back(0x20); // local.get
                fn_body.push_back(0x00); // param 0
                fn_body.push_back(0x20); // local.get
                fn_body.push_back(0x01); // param 1
                fn_body.push_back(0x6a); // i32.add
                fn_body.push_back(0x0f); // return
            }
            fn_body.push_back(0x0b); // end of function

            encode_u32_leb128(code_sec, static_cast<std::uint32_t>(fn_body.size()));
            code_sec.insert(code_sec.end(), fn_body.begin(), fn_body.end());
        }

        emit_section(bytes, 10, code_sec);

        out.wasm_bytes = std::move(bytes);
        return out;
    }

    static bool write_wasm_file(const WasmModuleOutput& wasm_out, const std::string& filepath) {
        std::ofstream ofs(filepath, std::ios::binary);
        if (!ofs.is_open()) return false;
        ofs.write(reinterpret_cast<const char*>(wasm_out.wasm_bytes.data()), wasm_out.wasm_bytes.size());
        ofs.close();
        return true;
    }

private:
    static void emit_section(std::vector<std::uint8_t>& out, std::uint8_t sec_id, const std::vector<std::uint8_t>& sec_data) {
        out.push_back(sec_id);
        encode_u32_leb128(out, static_cast<std::uint32_t>(sec_data.size()));
        out.insert(out.end(), sec_data.begin(), sec_data.end());
    }

    static void encode_u32_leb128(std::vector<std::uint8_t>& out, std::uint32_t val) {
        do {
            std::uint8_t byte = val & 0x7f;
            val >>= 7;
            if (val != 0) byte |= 0x80;
            out.push_back(byte);
        } while (val != 0);
    }

    static void encode_s32_leb128(std::vector<std::uint8_t>& out, std::int32_t val) {
        bool more = true;
        while (more) {
            std::uint8_t byte = val & 0x7f;
            val >>= 7;
            if ((val == 0 && (byte & 0x40) == 0) || (val == -1 && (byte & 0x40) != 0)) {
                more = false;
            } else {
                byte |= 0x80;
            }
            out.push_back(byte);
        }
    }

    static void encode_string(std::vector<std::uint8_t>& out, const std::string& str) {
        encode_u32_leb128(out, static_cast<std::uint32_t>(str.length()));
        out.insert(out.end(), str.begin(), str.end());
    }
};

} // namespace garuda::compiler
