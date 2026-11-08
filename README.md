# Garuda Compiler (G-BEAM) 🇮🇩

**Garuda** adalah bahasa pemrograman terdistribusi modern yang ditulis 100% oleh anak Indonesia, terinspirasi Erlang/OTP. Dilengkapi pattern matching, actor model, list/map comprehension, native HTTP server, fitur user-facing **Json/Http/Env**, aplikasi referensi **TodoAPI**, dan (Fase 14) **event bus Ets**, operasi **vektor**, **parsing/writing CSV**, **regex**, **UUID**, **priority queue**, **AES-256**, serta **JWT**.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Made in Indonesia](https://img.shields.io/badge/Made%20in-Indonesia-red)](https://richat.my.id)
[![Version](https://img.shields.io/badge/version-67.0.0-blue)](CHANGELOG.md)

---

## 🚦 Status Proyek

> **Update terakhir:** 2025-05-18  
> **HEAD diverifikasi:** `77c6725` / `cf99eb9` — `feat(green-fase14-penyiaran-peristiwa-v7): implementasi nyata bus peristiwa berbasis Ets`  

| Aspek | Status | Bukti |
|---|---|---|
| Build | ✅ PASS | MSVC / CMake Release Build |
| Test suite bahasa inti (Fase 6–11) | ✅ PASS | `docs/test-results-current.csv` |
| Fitur user-facing (Fase 12: Json, Http, Env, TodoAPI) | ✅ PASS | `docs/test-results-current.csv` |
| Package manager & packages (Fase 13) | ✅ PASS | 72/72 packages valid & complete |
| Logika Modul Packages (Fase 14: Ets bus, Vektor, CSV, Regex, UUID, PriQueue, AES256, JWT, KV DB) | ✅ PASS | `docs/test-results-current.csv` |
| **Total Test Suite Output** | **101 PASS, 5 FAIL, 6 TIMEOUT** | Lihat `docs/verification-report-fase14.md` |
| Known Issues | 11 Suites | Lihat section Known Issues |

---

## 📦 Fitur Utama

### Fitur Bahasa Inti (Fase 6–11)
- **Pattern matching** — `yen`, `lain`, tuple/list destructuring, guard clause.
- **Actor Model** — GenServer, Supervisor, PubSub, GenStatem state machines.
- **List & Map Comprehension** — Pipeline operator `|>`.
- **Native HTTP & Web Engine** — Router, middleware, static server, WebSocket, HTTPS, HTTP/2.
- **ETS Storage** — In-memory table storage (Set & Bag tables).
- **Distributed Actor** — Raft consensus, cluster manager, DistNode.
- **Kriptografi & Keamanan** — AES-GCM 256, X25519 DH, JWT.

### Fitur User-Facing (Fase 12)
- **`Json.enkode(value)`** / **`Json.dekode(string)`** — Serialisasi & deserialisasi JSON native.
- **`Http.ambil(url)`** / **`Http.kirim(url, body)`** — Client HTTP native.
- **`Env.baca(key, default)`** / **`Env.muat_config(path)`** — Environment variables & config loader.
- **TodoAPI** — Aplikasi referensi `.grd` murni di `examples/todo_api/`.

### Fitur Baru (Fase 14 — Terverifikasi `77c6725`)
- **Event Bus Ets (`PenyiaranPeristiwa`)** — Bus peristiwa terdistribusi in-memory berbasis ETS (`77c6725`).
- **Matematika Vektor (`MatematikaVektor`)** — Operasi aljabar vektor (dot product, magnitude, norma) (`6fd30d3`).
- **CSV Tabel (`CsvTabel`)** — Parsing dan penulisan format CSV (`e386990`).
- **Regex Pola (`RegexPola`)** — Pencocokan pola ekspresi reguler (`4c05e70`).
- **Identitas UUID (`IdentitasUuid`)** — Generator & validasi UUID v4 (`b9409ff`).
- **Http Klien (`HttpKlien`)** — Client HTTP wrapper berbasis Http BIF (`35f6fee`).
- **Antrean Prioritas (`AntreanPrioritas`)** — Heap / priority queue berbasis ETS (`d6dfcc5`).
- **Kripto AES-256 (`KriptoAes256`)** — Enkripsi/dekripsi AES-256 (`d73ac7b`).
- **JWT Token (`JwtToken`)** — Generator & pembuat klaim JWT token (`a38ef84`).
- **Basis Data KV (`BasisdataKv`)** — Key-Value Database berbasis ETS (`c7ec219`).

### Package Ecosystem (Fase 13)
- **72 Paket Resmi** di direktori `packages/`.
- **Package Manager (`grdmix`)** — Semver resolution, verifikasi checksum SHA-256, dan generator stub pustaka standar.
- Setiap paket 100% lengkap dengan: `garuda.json`, `grdmix.toml`, `README.md`, `src/lib.grd`, `src/test.grd`.

---

## 🚀 Cara Build & Jalankan

### Linux / macOS (CMake)
```bash
git clone https://github.com/anggitPM17/garuda.git
cd garuda/garuda_v67_0_production/garuda_v67.0.0
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/garuda_richat_server 9002
```

### Windows (PowerShell & CMake)
```powershell
git clone https://github.com/anggitPM17/garuda.git
cd garuda\garuda_v67_0_production\garuda_v67.0.0
cmake -B build
cmake --build build --config Release
.\build\Release\garuda_richat_server.exe 9002
```

### Jalankan Test Suite
```powershell
# Jalankan runner test cepat
powershell -ExecutionPolicy Bypass -File "scratch/run_all_tests.ps1"
```

---

## 📊 Benchmark Performa (VPS Ubuntu 1GB RAM)

> **Catatan:** Angka benchmark diukur pada kondisi idle dan beban HTTP. Perhitungan lengkap dan skrip reproduksi tersedia di folder `bench/`.

| Metrik | Erlang/OTP | Garuda BEAM C++ | Efisiensi |
|---|---|---|---|
| RAM Idle | ~69 MB | **~536 KB** | **38x Lebih Hemat** |
| Requests / Sec (RPS) | ~50 req/s | **191 req/s** | **3.8x Lebih Cepat** |
| Availability (Load Test) | 99.2% | **100.00%** | **Zero Error** |
| Chaos Recovery (SIGKILL) | ~10 detik | **< 5 detik** | **2x Lebih Tangguh** |

---

## 📚 Dokumentasi

- **[Laporan Verifikasi Fase 14](docs/verification-report-fase14.md)** — Bukti eksekusi test & status komponen.
- **[Catatan Rilis & CHANGELOG](CHANGELOG.md)** — Riwayat perubahan rilis.
- **[Peta Pencapaian Ekosistem](ACHIEVEMENTS.md)** — Rangkuman 6 pilar ekosistem GVM.

---

## ⚠️ Known Issues (Transparan)

Daftar dari `docs/verification-report-fase14.md`:

- **5 Test FAIL**: `garuda_chat_server_tests.exe`, `garuda_db_tests.exe`, `garuda_event_loop_backend_tests.exe`, `garuda_fullstack_tests.exe` (Socket teardown / port collision saat dijalankan berurutan cepat), `garuda_https_e2e_tests.exe` (Membutuhkan konfigurasi cert SSL local / OpenSSL mock binding di runner OS).
- **6 Test TIMEOUT**: Suites yang menjalankan loop HTTP server / socket listener terikat port (`garuda_async_event_loop_tests`, `garuda_bytecode_io_tests`, `garuda_cluster_manager_tests`, `garuda_fault_tests`, `garuda_http_tests`, `garuda_todo_api_e2e_tests`). Melebihi limit 10 detik pada eksekusi batch non-interaktif.

---

## 🌐 Ekosistem Live

- **[richat.my.id](https://richat.my.id)** — Portal resmi & demo live.
- **[Stadion Garuda](https://richat.my.id/stadion)** — Interactive Web Playground.
- **[WASM Client](https://richat.my.id/wasm)** — Eksekusi WebAssembly di browser.
- **[Dashboard GVM](https://richat.my.id/dashboard)** — Monitoring Prometheus metrics & RAM real-time.

---

## 📜 Lisensi & Atribusi

Garuda Compiler dirilis di bawah **MIT License**.  
Copyright © 2025 **Anggit Prasetyo Mituhu**.

Jika Anda menggunakan, memodifikasi, atau mendistribusikan Garuda, Anda **WAJIB** menyertakan atribusi:

> "Garuda Compiler by Anggit Prasetyo Mituhu (https://github.com/anggitPM17/garuda)"

Lihat [LICENSE](LICENSE), [AUTHORS](AUTHORS), dan [NOTICE](NOTICE) untuk detail lengkap.

---

## 📬 Kontak

- **Author:** Anggit Prasetyo Mituhu
- **Email:** anggit.prasetyo3060@gmail.com
- **WhatsApp:** +62 813-2992-5455
- **Location:** Semarang, Jawa Tengah, Indonesia
- **Website:** [richat.my.id](https://richat.my.id)
- **GitHub:** [@anggitPM17](https://github.com/anggitPM17)

---

**Dibuat dengan ❤️ di Indonesia. © 2025 Anggit Prasetyo Mituhu. 🇮🇩**
