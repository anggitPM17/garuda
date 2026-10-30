# 📜 AUDIT DUKUNGAN KATA KUNCI `impor` / `import` — G4 FASE A AUDIT REPORT (VERIFIED)

Dokumen ini mencatat hasil audit teknis ketersediaan dukungan impor modul/file (`impor` / `import`) pada compiler dan runtime **Garuda BEAM**.

---

## 📊 Hasil Audit Komponen Compiler & Runtime

| Komponen | Status | Bukti Kode (Path & Baris) | Catatan & Analisis |
| :--- | :---: | :--- | :--- |
| **Lexer (`TokenType::Import`)** | ❌ Belum Ada | `include/garuda/compiler/lexer.hpp` (baris 110–135) | Tokenized sebagai `TokenType::Identifier` ("impor"). |
| **Parser (`parse_import`)** | ❌ Belum Ada | `include/garuda/compiler/parser.hpp` (baris 30–50) | Belum ada parser khusus untuk `impor "file.grd"`. |
| **AST (`ImportNode`)** | ❌ Belum Ada | `include/garuda/compiler/ast.hpp` (baris 15–25) | `AstNodeType::ImportDecl` belum terdaftar di enum. |
| **Codegen (`ImportExpr`)** | ❌ Belum Ada | `include/garuda/compiler/codegen.hpp` (baris 230–250) | Belum ada handler penggabungan `function_table`. |
| **Runtime (`eval_file`)** | ⚠️ Parsial | `include/garuda/beam/tools/script_engine.hpp` (baris 58–63) | Memuat 1 file tunggal, mengembalikan `std::optional<Term>`. |
| **Modul AST (`ModuleNode`)** | ✅ Ada | `include/garuda/compiler/ast.hpp` (baris 492) | `ModuleNode` menyimpan nama modul & daftar fungsi. |
| **Contoh File `.grd`** | ❌ Belum Ada | `std/*.grd` & `examples/*.grd` | Belum ada file `.grd` yang menggunakan `impor`. |

---

## 🎯 Implikasi Arsitektur untuk FASE B

1. **Pemisahan Compile vs Execute (`compile_only`)**:
   * `eval_file(path)` mengembalikan `std::optional<Term>` (hasil eksekusi fungsi `utama`), sehingga tidak bisa langsung dipakai untuk impor modul.
   * Diperlukan fungsi pembantu `compile_module(path)` pada `ScriptEngine` / `CodeGenerator` yang mengompilasi file `.grd` terimpor menjadi `CodeGeneratorOutput` (tanpa mengeksekusi `utama`).

2. **Pendekatan AST Node vs Preprocessor**:
   * **Pendekatan A (AST Node Resmi)**:
     * Lexer: `TokenType::Import` untuk keyword `impor` / `import`.
     * Parser: `parse_import()` menghasilkan `ImportNode(path)`.
     * Codegen: `ImportNode` mengompilasi file target dan menggabungkan `function_table` + `all_instructions` dengan penyesuaian offset IP.
   * **Kelebihan Pendekatan A**: Bersih, AST-compliant, mendukung circular import prevention, dan siap untuk namespace isolation di G5.
