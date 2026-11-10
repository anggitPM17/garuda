#pragma once
// Garuda Language Compiler: Binary Bytecode Deserialization Reader (`bytecode_reader.hpp`)
// Membaca & memverifikasi berkas biner .gvm (Header, Magic "GVM1", Checksum, Function Table, & Code Section).
#include "codegen.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::compiler {

using namespace garuda::beam;

class BytecodeReader {
public:
    static std::optional<CodeGeneratorOutput> read(const std::string& filepath) {
        std::ifstream ifs(filepath, std::ios::binary);
        if (!ifs.is_open()) {
            std::cout << "⚠️ [GARUDA BYTECODE READER] Cannot open file: " << filepath << std::endl;
            return std::nullopt;
        }

        char header[32];
        ifs.read(header, 32);
        if (ifs.gcount() < 32) return std::nullopt;

        // 1. Verify Magic Number "GVM1"
        if (std::memcmp(header, "GVM1", 4) != 0) {
            std::cout << "⚠️ [GARUDA BYTECODE READER] Invalid Magic Number in: " << filepath << std::endl;
            return std::nullopt;
        }

        std::uint16_t version = read_uint16(header + 4);
        std::uint16_t mod_name_len = read_uint16(header + 8);
        std::uint16_t fn_count = read_uint16(header + 10);
        std::uint32_t inst_count = read_uint32(header + 12);
        std::uint32_t expected_checksum = read_uint32(header + 16);
        (void)mod_name_len;
        (void)fn_count;
        (void)inst_count;

        // Read remaining body
        std::string body((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        // 2. Verify Checksum
        std::uint32_t actual_checksum = fnv1a_32(body);
        if (expected_checksum != actual_checksum) {
            std::cout << "⚠️ [GARUDA BYTECODE READER] Checksum Mismatch! File corrupted or tampered (Expected: 0x"
                      << std::hex << expected_checksum << ", Actual: 0x" << actual_checksum << std::dec << ")" << std::endl;
            return std::nullopt;
        }

        std::istringstream iss(body, std::ios::binary);
        CodeGeneratorOutput out;

        // 3. Module Name
        std::uint16_t read_mod_len = read_stream_uint16(iss);
        std::vector<char> mod_buf(read_mod_len);
        iss.read(mod_buf.data(), read_mod_len);
        out.module_name = std::string(mod_buf.data(), read_mod_len);

        // 4. Function Table
        std::uint16_t read_fn_count = read_stream_uint16(iss);
        for (std::uint16_t i = 0; i < read_fn_count; ++i) {
            std::uint8_t name_len = static_cast<std::uint8_t>(iss.get());
            std::vector<char> fn_buf(name_len);
            iss.read(fn_buf.data(), name_len);
            std::string fn_name(fn_buf.data(), name_len);

            std::uint8_t arity = static_cast<std::uint8_t>(iss.get());
            std::uint32_t start_ip = read_stream_uint32(iss);

            CompiledFunctionInfo fn_info{fn_name, arity, start_ip, {}};
            out.functions.push_back(fn_info);

            std::string short_name = fn_name;
            auto dot_pos = fn_name.find('.');
            if (dot_pos != std::string::npos) short_name = fn_name.substr(dot_pos + 1);

            out.function_table[fn_name] = start_ip;
            out.function_table[fn_name + "/" + std::to_string(arity)] = start_ip;
            out.function_table[short_name] = start_ip;
            out.function_table[short_name + "/" + std::to_string(arity)] = start_ip;
            out.function_table[short_name] = start_ip;
        }

        // 5. Code Section
        std::uint32_t read_inst_count = read_stream_uint32(iss);
        for (std::uint32_t i = 0; i < read_inst_count; ++i) {
            Instruction inst;
            inst.op = static_cast<Opcode>(iss.get());
            std::uint8_t arg_count = static_cast<std::uint8_t>(iss.get());

            for (std::uint8_t a = 0; a < arg_count; ++a) {
                RegType reg_type = static_cast<RegType>(iss.get());
                std::uint16_t index = read_stream_uint16(iss);
                Term lit;
                if (reg_type == RegType::Literal) {
                    lit = deserialize_term(iss);
                }
                inst.args.push_back(Operand{reg_type, index, lit});
            }
            out.all_instructions.push_back(inst);
        }

        std::cout << "✨ [GARUDA BYTECODE READER] Successfully loaded " << out.module_name
                  << " v" << version << " (" << out.all_instructions.size() << " opcodes) from: " << filepath << std::endl;

        return out;
    }

private:
    static std::uint16_t read_uint16(const char* buf) {
        return (static_cast<std::uint8_t>(buf[0]) << 8) | static_cast<std::uint8_t>(buf[1]);
    }

    static std::uint32_t read_uint32(const char* buf) {
        return (static_cast<std::uint8_t>(buf[0]) << 24) |
               (static_cast<std::uint8_t>(buf[1]) << 16) |
               (static_cast<std::uint8_t>(buf[2]) << 8) |
               static_cast<std::uint8_t>(buf[3]);
    }

    static std::uint16_t read_stream_uint16(std::istream& is) {
        char buf[2];
        is.read(buf, 2);
        return read_uint16(buf);
    }

    static std::uint32_t read_stream_uint32(std::istream& is) {
        char buf[4];
        is.read(buf, 4);
        return read_uint32(buf);
    }

    static Term deserialize_term(std::istream& is) {
        std::uint8_t type = static_cast<std::uint8_t>(is.get());
        if (type == 1) { // Int
            std::uint64_t val = 0;
            for (int i = 0; i < 8; ++i) {
                val = (val << 8) | static_cast<std::uint8_t>(is.get());
            }
            return Term(static_cast<std::int64_t>(val));
        } else if (type == 2) { // Atom
            std::uint16_t len = read_stream_uint16(is);
            std::vector<char> buf(len);
            is.read(buf.data(), len);
            return Term(Atom{std::string(buf.data(), len)});
        } else if (type == 3) { // String
            std::uint16_t len = read_stream_uint16(is);
            std::vector<char> buf(len);
            is.read(buf.data(), len);
            return Term(std::string(buf.data(), len));
        }
        return Term();
    }

    static std::uint32_t fnv1a_32(const std::string& str) {
        std::uint32_t hash = 2166136261u;
        for (unsigned char c : str) {
            hash ^= c;
            hash *= 16777619u;
        }
        return hash;
    }
};

} // namespace garuda::compiler
