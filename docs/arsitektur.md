# 🦅 Arsitektur Mesin & Compiler Garuda (GVM)

Dokumen ini menjelaskan alur arsitektur lengkap dari kode sumber `.grd` hingga dieksekusi oleh Virtual Machine GVM.

---

## 1. Diagram Alur Eksekusi

```text
┌────────────────┐     ┌──────────────┐     ┌────────────────┐     ┌────────────────┐
│ Source (.grd)  │ ──> │ Lexer & Tok  │ ──> │ Recursive Parser│ ──>│ AST Tree Nodes │
└────────────────┘     └──────────────┘     └────────────────┘     └───────┬────────┘
                                                                           │
┌────────────────┐     ┌──────────────┐     ┌────────────────┐             │
│ Exec & Results │ <── │ GVM Runtime  │ <── │ Codegen Output │ <───────────┘
└────────────────┘     └──────────────┘     └────────────────┘
```

---

## 2. Komponen Utama Engine

1. **Lexer (`lexer.hpp`)**: Mengubah teks kode sumber menjadi alokasi Token dengan pelacakan baris dan kolom.
2. **Parser (`parser.hpp`)**: *Recursive Descent Parser* dengan *Precedence Climbing* 11 tingkat.
3. **AST Hierarchy (`ast.hpp`)**: Struktur data pohon abstrak terstruktur untuk modul, fungsi, percabangan, dan ekspresi aktor.
4. **Code Generator (`codegen.hpp`)**: Menelusuri AST, melakukan alokasi register X, dan menghasilkan instruksi *Opcode GVM*.
5. **Bytecode Serializer (`bytecode_writer.hpp` & `bytecode_reader.hpp`)**: Menyimpan dan membaca berkas biner `.gvm` 32-bytes header + FNV1a Checksum.
6. **GVM Interpreter (`interpreter.hpp`)**: Virtual Machine berbasis Register (X 256) & Call Stack Frame Isolation.
7. **Actor Runtime (`actor_runtime.hpp`)**: Penjadwal aktor $O(1)$ *Ready Queue*, *preemptive reduction counter* (4000/slice), dan per-process Generational GC.
