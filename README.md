# 🦅 Bahasa Garuda (v67.0.0)

> **Bahasa Pemrograman Fungsional & Actor-Model Indonesia dengan Garuda Virtual Machine (GVM). 100% Karya Anak Bangsa 🇮🇩**

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0)
[![Version](https://img.shields.io/badge/Version-v67.0.0-blue)](https://richat.my.id)
[![Build Status](https://img.shields.io/badge/Tests-14%2F14%20PASS%20(100%25)-brightgreen)](https://richat.my.id)

---

## 🌐 Ringkasan

**Bahasa Garuda** adalah bahasa pemrograman fungsional modern bergaya Erlang/Elixir yang dieksekusi di atas **Garuda Virtual Machine (GVM)**. Bahasa Garuda dirancang untuk aplikasi terdistribusi, *actor concurrency*, *web backend*, dan *system tooling* berlatensi ultra-rendah dengan konsumsi memori yang sangat efisien.

Bahasa Garuda v67.0.0 hadir dengan ekosistem lengkap: CLI Tooling (`grd`), Web Framework (`std/web.grd`), Language Server Protocol (LSP), serta Ekstensi VS Code.

---

## ✨ Fitur Utama

- **⚡ Fungsional & Ekspresif**: First-class functions, lambda anonim, pattern matching, guard clauses (`nalika`), dan pipe operator (`|>`).
- **🎭 Actor Model Concurrency**: `cakra` (spawn process), `kirim` (send message), `tampa` (receive), dan `diri` (self PID).
- **🛡️ Guard Clause (`nalika`)**: Validasi parameter fungsi secara deklaratif sebelum fungsi dieksekusi.
- **🔗 Pipe Operator (`|>`)**: Komposisi fungsi yang bersih tanpa nested function calls.
- **📚 Pustaka Standar Murni**: List, String, Map, Tuple, Json, Waktu, Matematika, HTTP Client, dan Web Framework.
- **🌐 Web Framework**: Routing dinamis GET/POST dan eksekusi handler Garuda penuh yang thread-safe.
- **🛠️ Tooling & IDE Support**: CLI Tooling (`grd`), LSP Server (`garuda-lsp`), dan Ekstensi VS Code (`garuda-vscode-lsp`).
- **✅ 100% Test Pass Rate**: 14/14 test suite lulus uji (0 Fail, 0 Skip) termasuk isu `List.peta` + lambda.

---

## 📊 Benchmark Performa Nyata

Diukur pada Windows 10 (MSVC 2022 x64, Build Release), 3 iterasi eksekusi (nilai Median). Laporan lengkap pada **[benchmarks/RESULTS.md](benchmarks/RESULTS.md)**.

| Kategori Benchmark | Deskripsi Pengujian | Median (ms) | Min (ms) | Max (ms) |
| :--- | :--- | :---: | :---: | :---: |
| **Startup Engine** | Evaluation & execution `hello_world` | **0.032 ms** | 0.024 ms | 0.360 ms |
| **Actor Concurrency** | `cakra` spawn process + `kirim` pesan | **0.071 ms** | 0.067 ms | 0.271 ms |
| **Map Operations** | `Map.taruh` multi-entry + `Map.ambil` | **1.086 ms** | 0.935 ms | 1.216 ms |
| **String Manipulation** | `String.ganti` + `String.balik` (17 std/string) | **1.538 ms** | 1.298 ms | 1.543 ms |
| **Functional List** | `List.peta` dengan lambda anonim | **4.236 ms** | 4.076 ms | 4.478 ms |
| **JSON Serialization** | `Json.encode_list` + `Json.decode_list` | **7.661 ms** | 7.078 ms | 11.251 ms |

---

## 🚀 Quick Start

### 1. Kompilasi CLI `grd` (Windows MSVC / CMake)

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target grd
```

### 2. Program Pertama (Hello World)

Buat berkas `halo.grd`:
```garuda
modul Halo

fungsi utama() ->
    cetak("Halo dari Bahasa Garuda!")
rampung
```

Jalankan skrip menggunakan `grd`:
```powershell
.\build\Release\grd.exe run halo.grd
# Output: => "Halo dari Bahasa Garuda!"
```

### 3. Buat Proyek Baru

```powershell
.\build\Release\grd.exe new proyek_saya
cd proyek_saya
..\build\Release\grd.exe run utama.grd
```

### 4. Jalankan Seluruh Test Suite

```powershell
.\build\Release\grd.exe test
# Output: RINGKASAN HASIL TEST: 14 PASS | 0 FAIL | 0 SKIP
```

---

## 📐 Struktur Repositori

```text
garuda_v67.0.0/
├── src/                  # Kode Sumber C++ (grd.cpp, grdc.cpp, gvm_admin.cpp)
├── include/              # Header C++ GVM & Compiler
│   └── garuda/
│       ├── beam/tools/script_engine.hpp   # Runtime Interpreter & BIFs
│       └── compiler/codegen.hpp            # Code Generator & Bytecode
├── std/                  # Pustaka Standar Bahasa Garuda (.grd)
│   ├── list.grd, string.grd, map.grd, tuple.grd
│   ├── json.grd, waktu.grd, matematika.grd
│   ├── http.grd, http_server.grd, web.grd
├── tests/                # Test Suite C++ (14 Executables)
├── lsp/                  # Language Server Protocol (Node.js / TypeScript)
├── vscode-extension/     # Ekstensi Client VS Code
├── docs/                 # Dokumentasi Lengkap
└── examples/             # Contoh Program Nyata
```

---

## 📚 Pustaka Standar

| Modul | Berkas | Fungsi Utama | Status |
| :--- | :--- | :--- | :---: |
| **List** | `std/list.grd` | `panjang`, `kepala`, `ekor`, `tambah`, `balik`, `anggota`, `peta`, `saring`, `lipat` | ✅ |
| **String** | `std/string.grd` | 17 fungsi (`panjang`, `potong`, `pisah`, `ganti`, `cari`, `mulai_dengan`, `akhir_dengan`, `balik`, `ulang`, `ke_besar_awal`, `rapikan`, dll) | ✅ |
| **Map** | `std/map.grd` | 10 fungsi (`baru`, `taruh`, `ambil`, `hapus`, `ukuran`, `mengandung_kunci`, `kosong`, `gabung`) | ✅ |
| **Tuple** | `std/tuple.grd` | `elemen`, `panjang`, `ke_list`, `gabung` | ✅ |
| **JSON** | `std/json.grd` | `encode_int`, `decode_int`, `encode_string`, `decode_string`, `encode_bool`, `decode_bool`, `encode_list`, `decode_list`, `encode_map`, `decode_map` | ✅ |
| **Waktu** | `std/waktu.grd` | `sekarang`, `tunggu`, `durasi_mulai`, `durasi_selesai` | ✅ |
| **Matematika** | `std/matematika.grd` | `pangkat`, `abs`, `min`, `max`, `akar`, `pembulatan` | ✅ |
| **HTTP Client** | `std/http.grd` | `ambil` (GET), `kirim_json` (POST) | ✅ |
| **HTTP Server** | `std/http_server.grd` | `mulai`, `stop` | ✅ |
| **Web Framework** | `std/web.grd` | `daftar_route`, `get`, `post`, `daftar_handler`, `jalankan` | ✅ |

---

## 💡 Contoh Program

### 1. Fungsional dengan `List.peta` & `List.saring`
```garuda
modul ContohFungsional
impor "std/list.grd"

fungsi utama() ->
    Angka = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    Genap = List.saring(Angka, fungsi(X) -> X % 2 == 0 rampung)
    Hasil = List.peta(Genap, fungsi(X) -> X * 10 rampung)
    cetak(Hasil)  -- [20, 40, 60, 80, 100]
rampung
```

### 2. Web Framework REST Server
```garuda
modul WebApp
impor "std/web.grd"

fungsi utama() ->
    Web.daftar_handler("h_halo", "\"Halo dari Bahasa Garuda Web Framework!\"")
    Web.daftar_handler("h_hitung", "40 + 2")
    Web.get("/halo", "h_halo")
    Web.get("/hitung", "h_hitung")
    Web.jalankan(8080)
rampung
```

### 3. Concurrency Model Actor (`cakra`)
```garuda
modul ContohAktor

fungsi utama() ->
    Pid = cakra fungsi() ->
        tampa
            {halo, Teks} -> cetak("Pesan Diterima: " ++ Teks)
            _ -> cetak("Pesan Lain")
        rampung
    rampung

    kirim(Pid, {halo, "Garuda BEAM"})
rampung
```

### 4. Contoh Aplikasi Nyata (`examples/`)

| Aplikasi Contoh | Deskripsi & Fitur | Cara Menjalankan |
| :--- | :--- | :--- |
| 🌐 **[todo_api](examples/todo_api/)** | RESTful Web API Todo List (`std/web.grd`, `std/json.grd`, `std/map.grd`) | `grd run examples/todo_api/utama.grd` |
| 💬 **[chat_server](examples/chat_server/)** | Chat Server dengan Actor Concurrency & Web Endpoint | `grd run examples/chat_server/utama.grd` |
| 📊 **[data_analysis](examples/data_analysis/)** | Pipeline Analisis Data Nilai Siswa (`List.saring`, `List.peta`) | `grd run examples/data_analysis/utama.grd` |
| ⚙️ **[json_parser](examples/json_parser/)** | Deserialisasi & Pemrosesan Format JSON (`std/json.grd`) | `grd run examples/json_parser/utama.grd` |
| 🧮 **[calculator](examples/calculator/)** | Kalkulator Fungsional dengan Pattern Matching & Guard Clause | `grd run examples/calculator/utama.grd` |

---

## 🛠️ Tooling & IDE Support

### CLI Tooling (`grd`)
- `grd run <file.grd>` — Mengeksekusi skrip Garuda.
- `grd test` — Jalankan 14 suite pengujian C++ secara otomatis.
- `grd new <nama>` — Membuat struktur proyek Garuda baru.
- `grd --help` — Menampilkan informasi bantuan.
- `grd --version` — Menampilkan versi runtime.

### LSP & VS Code Extension
Lihat panduan lengkap pada [docs/lsp-vscode.md](docs/lsp-vscode.md) untuk:
- Autocomplete 68 simbol (21 keyword & 47 fungsi stdlib).
- Hover documentation Markdown real-time.
- Diagnostik sintaks kesalahan kurung/string.

---

## 🧪 Suite Pengujian (`grd test`)

Seluruh 14 test suite lulus 100% tanpa error:
```text
======================================================
RINGKASAN HASIL TEST: 14 PASS | 0 FAIL | 0 SKIP
======================================================
```

1. **Waktu & Durasi**
2. **Matematika**
3. **HTTP Client (GET/POST)**
4. **HTTP Server**
5. **Web Handler Garuda**
6. **String Module** (21 tests)
7. **Map Module** (8 tests)
8. **Tuple Module** (6 tests)
9. **Interpolasi String**
10. **Guard Clause (`nalika`)**
11. **Pipe Operator (`|>`)**
12. **Actor Concurrency** (12 assertions)
13. **JSON Encoder/Decoder**
14. **Isolate Binop (KB-001)** — `List.peta` / `List.saring` + lambda **PASS**

---

## 📖 Dokumentasi Lengkap

- 📖 **[Sintaks Bahasa (docs/sintaks.md)](docs/sintaks.md)** — Panduan sintaks dan keyword.
- 📦 **[Pustaka Standar (docs/pustaka-standar.md)](docs/pustaka-standar.md)** — Referensi fungsi `std/*.grd`.
- 🌐 **[Web Framework (docs/web-framework.md)](docs/web-framework.md)** — Panduan backend web.
- 🎨 **[LSP & VS Code (docs/lsp-vscode.md)](docs/lsp-vscode.md)** — Penggunaan IDE & Autocomplete.
- 🏛️ **[Arsitektur GVM (docs/arsitektur.md)](docs/arsitektur.md)** — Desain internal compiler & interpreter.
- 📂 **[Contoh Program (examples/)](examples/)** — Kode contoh yang siap dijalankan.

---

## 📜 Lisensi & Kontribusi

Proyek ini dilesensikan di bawah **[Apache License 2.0](LICENSE)** — bebas digunakan, dimodifikasi, dan didistribusikan untuk keperluan personal maupun komersial, dengan tetap mempertahankan atribusi dan perlindungan paten bawaan lisensi.

Hak Cipta (c) 2026 **Anggit Prasetyo Mituhu**. Lihat berkas **[NOTICE](NOTICE)** untuk informasi atribusi.

Panduan kontribusi dapat dibaca pada **[CONTRIBUTING.md](CONTRIBUTING.md)**. Silakan ajukan *Issue* atau *Pull Request* di repositori resmi **[GitHub anggitPM17/garuda](https://github.com/anggitPM17/garuda)**.

---
**Dibuat dengan ❤️ di Indonesia. &copy; 2026 Anggit Prasetyo Mituhu. 🇮🇩**
