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
| **Fase 0** | Fondasi Handler Parameter & Standard Library (`set`, `queue`, `stack`, `file`, `web`) | ✅ **PASS** | 100% PASS (19/19 Test) |
| **Fase 1** | Type System (Annotation, Checking, Sum Types, Generic, System) | ✅ **PASS** | 100% PASS (13/13 Test) |
| **Fase 2** | Compiler Murni dalam Garuda (`lexer.grd`, `parser.grd`, `codegen.grd`, `grdc.grd`) | ✅ **PASS** | 100% PASS (8/8 Test) |
| **Fase 3** | Virtual Machine dalam Garuda (`vm_memory.grd`, `vm_eval.grd`, `vm_bif.grd`, `vm_actor.grd`, `grdrun.grd`) | ✅ **PASS** | 100% PASS (11/11 Test) |
| **Fase 4** | Bootstrap End-to-End (`Stage 1 == Stage 2`) | ✅ **PASS** | 100% PASS (15/15 Test) |
| **Fase 5** | Fitur Lanjutan, Optimasi Register, Benchmark & Documentation Final | 🔄 **Dalam Pengembangan** | Fitur Sintaksis (Pipe, Try/Catch, Guard, Send) PASS |

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

---

## 🤝 Panduan Kontribusi

Saat memperbarui modul di `std/` atau komponen bootstrap di `src/`:
1. Pastikan **seluruh test suite** di atas lulus 100% tanpa regresi.
2. Tambahkan test case baru di folder `tests/` jika menambah fungsi baru di `std/*.grd`.
3. Pertahankan kompatibilitas mundur dengan BIF eksekusi GVM C++.
