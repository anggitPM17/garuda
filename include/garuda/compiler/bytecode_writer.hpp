#pragma once
// Garuda Language Compiler: Binary Bytecode Serialization Writer (`bytecode_writer.hpp`)
// Mengubah CodeGeneratorOutput menjadi berkas biner .gvm dengan Header, Atom Table, Function Table, Code Section, & CRC32.
#include "codegen.hpp"
#include "../beam/crypto/crypto.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace garuda::compiler {

using namespace garuda::beam;

class BytecodeWriter {
public:
    static bool write(const CodeGeneratorOutput& out, const std::string& filepath) {
        std::ofstream ofs(filepath, std::ios::binary);
        if (!ofs.is_open()) return false;

        // Serialize body stream first to calculate CRC32 checksum
        std::ostringstream body;

        // 1. Module Name
        std::uint16_t mod_name_len = static_cast<std::uint16_t>(out.module_name.length());
        write_uint16(body, mod_name_len);
        body.write(out.module_name.data(), mod_name_len);

        // 2. Function Table
        std::uint16_t fn_count = static_cast<std::uint16_t>(out.functions.size());
        write_uint16(body, fn_count);
        for (const auto& fn : out.functions) {
            std::uint8_t name_len = static_cast<std::uint8_t>(fn.name.length());
            body.put(static_cast<char>(name_len));
            body.write(fn.name.data(), name_len);
            body.put(static_cast<char>(fn.arity));
            write_uint32(body, static_cast<std::uint32_t>(fn.start_ip));
        }

        // 3. Code Section
        std::uint32_t inst_count = static_cast<std::uint32_t>(out.all_instructions.size());
        write_uint32(body, inst_count);
        for (const auto& inst : out.all_instructions) {
            body.put(static_cast<char>(inst.op));
            body.put(static_cast<char>(inst.args.size()));
            for (const auto& arg : inst.args) {
                body.put(static_cast<char>(arg.type));
                write_uint16(body, static_cast<std::uint16_t>(arg.index));
                if (arg.type == RegType::Literal) {
                    serialize_term(body, arg.literal);
                }
            }
        }

        std::string body_str = body.str();
        std::uint32_t checksum = fnv1a_32(body_str);

        // 4. Header (32 bytes)
        ofs.write("GVM1", 4);                             // Magic: 4 bytes
        write_uint16(ofs, 1);                             // Version: uint16 = 1
        write_uint16(ofs, 0);                             // Flags: uint16 = 0
        write_uint16(ofs, mod_name_len);                  // Module Name Length
        write_uint16(ofs, fn_count);                      // Function Count
        write_uint32(ofs, inst_count);                    // Instruction Count
        write_uint32(ofs, checksum);                      // Checksum CRC32/FNV1a
        char reserved[12] = {0};                          // Reserved 12 bytes
        ofs.write(reserved, 12);

        // Write body
        ofs.write(body_str.data(), body_str.size());
        ofs.close();

        std::cout << "💾 [GARUDA BYTECODE WRITER] Written " << (32 + body_str.size())
                  << " bytes binary .gvm file: " << filepath << " (Checksum: 0x" << std::hex << checksum << std::dec << ")" << std::endl;

        return true;
    }

private:
    static void write_uint16(std::ostream& os, std::uint16_t val) {
        char buf[2];
        buf[0] = static_cast<char>((val >> 8) & 0xFF);
        buf[1] = static_cast<char>(val & 0xFF);
        os.write(buf, 2);
    }

    static void write_uint32(std::ostream& os, std::uint32_t val) {
        char buf[4];
        buf[0] = static_cast<char>((val >> 24) & 0xFF);
        buf[1] = static_cast<char>((val >> 16) & 0xFF);
        buf[2] = static_cast<char>((val >> 8) & 0xFF);
        buf[3] = static_cast<char>(val & 0xFF);
        os.write(buf, 4);
    }

    static void serialize_term(std::ostream& os, const Term& t) {
        if (auto* i = t.get_if<std::int64_t>()) {
            os.put(1); // Type Int
            std::uint64_t val = static_cast<std::uint64_t>(*i);
            for (int b = 56; b >= 0; b -= 8) os.put(static_cast<char>((val >> b) & 0xFF));
        } else if (auto* a = t.get_if<Atom>()) {
            os.put(2); // Type Atom
            std::uint16_t len = static_cast<std::uint16_t>(a->name.length());
            write_uint16(os, len);
            os.write(a->name.data(), len);
        } else if (auto* s = t.get_if<std::string>()) {
            os.put(3); // Type String
            std::uint16_t len = static_cast<std::uint16_t>(s->length());
            write_uint16(os, len);
            os.write(s->data(), len);
        } else {
            os.put(0); // Type Nil / Monostate
        }
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
