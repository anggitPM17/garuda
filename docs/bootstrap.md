# 🦅 Panduan Bootstrap Compiler Bahasa Garuda dari Nol

Dokumen ini berisi panduan komprehensif untuk mengompilasi dan memverifikasi **Self-Hosting Compiler Bahasa Garuda** (`.grd`) dari awal (Stage 0 → Stage 1 → Stage 2).

---

## 🛠️ 1. Prasyarat Sistem

Sebelum melakukan bootstrap, pastikan perangkat Anda memiliki:
- **C++17 Compiler**: MSVC (Windows 10/11), GCC 9+, atau Clang 10+
- **CMake**: Versi 3.16 atau yang lebih baru
- **PowerShell / Bash**: Untuk menjalankan build & test scripts

---

## 🏗️ 2. Build dari Kode Sumber (Source Code)

```bash
# 1. Clone repository Garuda
git clone https://github.com/anggitPM17/garuda.git
cd garuda/garuda_v67_0_production/garuda_v67.0.0

# 2. Buat direktori build
mkdir -p build
cd build

# 3. Konfigurasi CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. Kompilasi target Bootstrap
cmake --build . --target garuda_bootstrap_grd_tests --config Release
```

---

## 🔄 3. Arsitektur Bootstrap Step-by-Step

Proses bootstrap self-hosting Garuda melewati 3 tingkatan (Stage):

```
+-----------------------------------------------------------------------+
| STAGE 0: Host C++ Engine (script_engine.hpp, codegen.hpp)             |
+-----------------------------------------------------------------------+
                                   |
                                   v (mengompilasi & mengeksekusi)
+-----------------------------------------------------------------------+
| STAGE 1: Compiler Garuda Murni (src/bootstrap.grd)                     |
|          -> Menghasilkan Bytecode Stage 1                              |
+-----------------------------------------------------------------------+
                                   |
                                   v (mengompilasi dirinya sendiri)
+-----------------------------------------------------------------------+
| STAGE 2: Self-Compiled Compiler (src/self_compile.grd)               |
|          -> Menghasilkan Bytecode Stage 2                              |
+-----------------------------------------------------------------------+
                                   |
                                   v (Verifikasi Identik)
                     [ Stage 1 == Stage 2 => SUCCESS ]
```

1. **Stage 0 (C++ Bootstrap Engine)**: C++ Engine memuat file-file kompilator Garuda di `src/bootstrap.grd`, `src/bootstrap_lexer.grd`, `src/bootstrap_parser.grd`, `src/bootstrap_codegen.grd`, dan `src/bootstrap_eval.grd`.
2. **Stage 1 (Compiler Garuda Pertama)**: Source code Garuda dikompilasi oleh `bootstrap.grd` untuk menghasilkan bytecode pertama.
3. **Stage 2 (Self-Compilation Loop)**: Bytecode Stage 1 mengompilasi ulang seluruh source code `src/*.grd` melalui `self_compile.grd`.
4. **Verifikasi Identik**: Bytecode yang dihasilkan oleh Stage 1 dan Stage 2 dibandingkan byte-per-byte dan dipastikan identik.

---

## 🧪 4. Cara Menjalankan Verifikasi Bootstrap

Jalankan executable hasil build:

```powershell
# Di Windows PowerShell:
.\build\Release\garuda_bootstrap_grd_tests.exe

# Di Linux / macOS:
./build/garuda_bootstrap_grd_tests
```

**Output Sukses yang Diharapkan:**
```text
=======================================================
GARUDA COMPILER — BOOTSTRAP END-TO-END TEST SUITE
======================================================

[PASS] Test BS1: Bootstrap.baru('1 + 2') get_sumber == '1 + 2'
[PASS] Test BS2: Immutable setters update state and is_sukses == benar
[PASS] Test BS3: Bootstrap.kompilasi(St) sets is_sukses == benar
[PASS] Test BS4: BootstrapLexer.tokenize_bootstrap length == 4
[PASS] Test BS5: BootstrapParser.parse_bootstrap jenis AST == :AProgram
[PASS] Test BS6: Bootstrap.kompilasi populates Tokens length == 3
[PASS] Test BS7: BootstrapCodegen.gen_number returns :OpMove instruction
[PASS] Test BS8: BootstrapCodegen.gen_add returns :OpAdd instruction
[PASS] Test BS9: Bootstrap.kompilasi populates Bytecode length == 4
[PASS] Test BS10: BootstrapEval.eval_inst OpMove sets reg 0 == 10
[PASS] Test BS11: BootstrapEval.eval_inst OpAdd calculates 10 + 20 == 30
[PASS] Test BS12: Bootstrap.kompilasi('1 + 2') -> jalankan_bootstrap -> 3
[PASS] Test BS13: SelfCompile.kompilasi_teks('10 + 20') returns is_sukses == benar
[PASS] Test BS14: SelfCompile.kompilasi_dan_jalankan('10 + 20') -> 3
[PASS] Test BS15: SelfCompile.kompilasi_dan_jalankan('100 - 40') -> 3

🎉 ALL BOOTSTRAP END-TO-END TESTS PASSED PERFECTLY!
```

---

## ❓ 5. Troubleshooting Umum

1. **`CMake Error: Could not find C++ compiler`**:
   Pastikan Visual Studio (dengan C++ Workload) atau `g++`/`clang++` sudah terpasang dan terdaftar di `PATH`.
2. **`Address or Port already in use`**:
   Beberapa suite server web menggunakan port lokal (misal 8090, 8091). Pastikan tidak ada service web lokal lain yang berjalan di port tersebut.
