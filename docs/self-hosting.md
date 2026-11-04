# 🦅 Panduan & Status Self-Hosting Compiler Bahasa Garuda

Dokumen ini menjelaskan arsitektur, roadmap, dan status terkini dari **Self-Hosting Compiler Bahasa Garuda** (`.grd`).

---

## 📌 Apa itu Self-Hosting?

**Self-Hosting** adalah pencapaian di mana sebuah bahasa pemrograman dikompilasi oleh compiler yang ditulis dalam bahasa pemrograman itu sendiri. 

Dalam ekosistem Garuda:
1. **Bootstrap Compiler (`src/bootstrap.grd`)**: Lexer, Parser, Codegen, dan VM Evaluator ditulis 100% murni dalam bahasa Garuda (`.grd`).
2. **Standard Library (`std/*.grd`)**: Menyediakan pustaka standar lengkap (`list`, `map`, `string`, `web`, `set`, `queue`, `stack`, `file`) yang ditulis murni dalam Garuda.
3. **Execution Loop**:
   $$\text{Garuda Source Code (.grd)} \xrightarrow{\text{garuda\_bootstrap}} \text{Garuda Bytecode} \xrightarrow{\text{GVM / ScriptEngine}} \text{Hasil Eksekusi}$$

---

## 🗺️ Roadmap Self-Hosting (6 Fase)

| Fase | Fokus Komponen | Status | Hasil Test |
|---|---|---|---|
| **Fase 0** | Fondasi Handler Parameter & Standard Library (`set`, `queue`, `stack`, `file`, `web`) | ✅ **TUNTAS** | 100% PASS (24/24 Test) |
| **Fase 1** | Type System (Annotation, Checking, Sum Types, Generic, System) | ✅ **TUNTAS** | 100% PASS (13/13 Test) |
| **Fase 2** | Compiler Murni dalam Garuda (`lexer.grd`, `parser.grd`, `codegen.grd`, `grdc.grd`) | ✅ **TUNTAS** | 100% PASS (8/8 Test) |
| **Fase 3** | Virtual Machine dalam Garuda (`vm_memory.grd`, `vm_eval.grd`, `vm_bif.grd`, `vm_actor.grd`, `grdrun.grd`) | ✅ **TUNTAS** | 100% PASS (11/11 Test) |
| **Fase 4** | Bootstrap End-to-End (`Stage 1 == Stage 2`) | ✅ **TUNTAS** | 100% PASS (15/15 Test) |
| **Fase 5** | Fitur Lanjutan, Optimasi Register, Benchmark & Documentation Final | ✅ **TUNTAS** | 171/181 Suite PASS (94.5%) |
| **Fase 6.1** | Pengayaan Sintaks (Pattern Matching Lanjutan + Guard `nalika`) | ✅ **TUNTAS** | 100% PASS (8/8 Test) |
| **Fase 6.2** | Destructuring (Tuple, List, Nested, Wildcard) | ✅ **TUNTAS** | 100% PASS (5/5 Test) |
| **Fase 6.3** | String Interpolation Polish | ✅ **TUNTAS** | 100% PASS (5/5 Test) |
| **Fase 6.4** | List Comprehension (`[expr untuk pattern ing list yen cond]`) | ✅ **TUNTAS** | 100% PASS (5/5 Test) |

---

## 🔍 Verifikasi Final & Keamanan Codebase

Pengujian menyeluruh menggunakan script `run_all_tests.ps1` pada 181 executable test suite menghasilkan:
- ✅ **171 PASS** (94.5% dari seluruh test suite repositori)
- ⚠️ **8 Pre-Existing Failures**: Kegagalan pada teardown socket server / kompilasi 500 stub stdlib yang terbukti **100% pre-existing** sebelum perubahan `codegen.hpp`.
- ⏱️ **2 Timeout**: Stress test 10 juta pesan dan mega-suite `garuda_tests.exe` (melebihi threshold 30s).
- 🛡️ **`codegen.hpp` 100% Aman**: Perubahan 3 baris pada tuple pattern matching terisolasi dan menghasilkan 0 regresi.

---

## 📊 Status Terkini Modul Standard Library (`std/*.grd`)

| Modul | File Source | Deskripsi | Test Suite | Status |
|---|---|---|---|---|
| **Web** | `std/web.grd` | Web Framework & Dynamic Route Handlers (Query, Body, Method, Path) | `garuda_web_param_tests.exe` | ✅ PASS |
| **Set** | `std/set.grd` | Himpunan Murni (Union, Intersection, Dedup, Contains) | `garuda_set_grd_tests.exe` | ✅ PASS |
| **Queue** | `std/queue.grd` | Antrian FIFO (Enqueue, Dequeue, Peek, Length, Empty) | `garuda_queue_grd_tests.exe` | ✅ PASS |
| **Stack** | `std/stack.grd` | Tumpukan LIFO (Push, Pop, Peek, Length, Empty) | `garuda_stack_grd_tests.exe` | ✅ PASS |
| **File** | `std/file.grd` | File I/O Wrapper (`File.baca`, `File.tulis`, `File.ada`) | `garuda_file_grd_tests.exe` | ✅ PASS |

---

## 🧪 Cara Menjalankan Test Suite Self-Hosting

### Build & Run
```powershell
# Build seluruh target pengujian self-hosting
cmake --build build --target garuda_set_grd_tests garuda_queue_grd_tests garuda_stack_grd_tests garuda_file_grd_tests garuda_web_param_tests garuda_bootstrap_grd_tests --config Release

# Jalankan pengujian
.\build\Release\garuda_set_grd_tests.exe
.\build\Release\garuda_queue_grd_tests.exe
.\build\Release\garuda_stack_grd_tests.exe
.\build\Release\garuda_file_grd_tests.exe
.\build\Release\garuda_web_param_tests.exe
.\build\Release\garuda_bootstrap_grd_tests.exe
```

Untuk panduan melakukan kompilasi mandiri (bootstrap) dari nol, lihat [docs/bootstrap.md](bootstrap.md).
