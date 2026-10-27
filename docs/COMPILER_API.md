# 🛠️ PANDUAN INTEGRASI C++ COMPILER API GARUDA (`COMPILER_API.md`)

Dokumen spesifikasi integrasi C++ API compiler Garuda untuk menyematkan (*embedding*) compiler `grdc` ke dalam aplikasi C++ eksternal.

---

## 🎯 1. Ringkasan API

Compiler Garuda dirancang sebagai pustaka C++ header-only terstruktur yang dapat disematkan langsung ke dalam proyek C++ tanpa dependensi eksternal.

### Header Utama
```cpp
#include "garuda/compiler/lexer.hpp"
#include "garuda/compiler/parser.hpp"
#include "garuda/compiler/codegen.hpp"
```

---

## 💻 2. Contoh Kode Integrasi C++ Minimal

```cpp
#include "garuda/compiler/lexer.hpp"
#include "garuda/compiler/parser.hpp"
#include "garuda/compiler/codegen.hpp"
#include <iostream>

using namespace garuda::compiler;

int main() {
    std::string source_code = R"(
        modul Contoh
        fungsi utama() ->
            42
        rampung
    )";

    // 1. Tokenisasi
    Lexer lexer(source_code);
    auto tokens = lexer.tokenize();

    // 2. Parsing AST
    Parser parser(tokens);
    auto ast = parser.parse_program();

    // 3. Generasi Bytecode
    CodeGenerator codegen;
    CodeGeneratorOutput output = codegen.generate(ast);

    std::cout << "Kompilasi Berhasil! Modul: " << output.module_name 
              << ", Opcodes: " << output.all_instructions.size() << std::endl;
    return 0;
}
```

---

## 📄 3. Struktur `CodeGeneratorOutput`

| Field | Tipe Data | Deskripsi |
| --- | --- | --- |
| `module_name` | `std::string` | Nama modul yang dideklarasikan. |
| `all_instructions` | `std::vector<Instruction>` | Urutan instruksi opcodes bytecode GVM. |
| `function_table` | `std::unordered_map<std::string, std::size_t>` | Pemetaan nama fungsi ke offset instruksi (IP). |
| `functions` | `std::vector<CompiledFunctionInfo>` | Daftar metadata fungsi terkompilasi. |
