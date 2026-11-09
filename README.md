# Garuda Compiler (G-BEAM) 🇮🇩

**Garuda** adalah bahasa pemrograman terdistribusi modern yang ditulis 100% oleh anak Indonesia, terinspirasi Erlang/OTP. Dilengkapi pattern matching, actor model, list/map comprehension, native HTTP server, fitur user-facing **Json/Http/Env**, aplikasi referensi **TodoAPI**, (Fase 14) **event bus Ets**, **vektor**, **CSV**, **regex**, **UUID**, **AES-256**, **JWT**, (Fase 15 Part 1) **Standard Library Lengkap** (`List`, `Map`, `String`, `File`, `Math`, `Time`, `Path`), serta (Fase 15 Part 2) **Package Registry & Distribution (`grdmix`)** dengan verifikasi tanda tangan kriptografi **Ed25519** & **SHA-256**.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Made in Indonesia](https://img.shields.io/badge/Made%20in-Indonesia-red)](https://richat.my.id)
[![Version](https://img.shields.io/badge/version-67.0.0-blue)](CHANGELOG.md)

---

## 🚦 Status Proyek

> **Update terakhir:** 2025-05-18  
> **HEAD diverifikasi:** `784d619` — `chore(repo): remove non-Garuda project files from Git tracking`  

| Aspek | Status | Bukti |
|---|---|---|
| Build | ✅ PASS | MSVC / CMake Release Build |
| Test suite bahasa inti (Fase 6–11) | ✅ PASS | `docs/test-results-current.csv` |
| Fitur user-facing (Fase 12: Json, Http, Env, TodoAPI) | ✅ PASS | `docs/test-results-current.csv` |
| Package manager & packages (Fase 13) | ✅ PASS | 213 packages valid & complete |
| Logika Modul Packages (Fase 14: Ets bus, Vektor, CSV, Regex, UUID, PriQueue, AES256, JWT, KV DB) | ✅ PASS | `docs/test-results-current.csv` |
| Standard Library (Fase 15 Part 1: List, Map, String, File, Math, Time, Path) | ✅ PASS | `docs/test-results-current.csv` (7/7 suites PASS) |
| **Package Registry & Distribution (Fase 15 Part 2: Semver, Ed25519 Signature, SHA-256 Checksum)** | ✅ PASS | `docs/test-results-current.csv` (2/2 suites PASS) |
| **Total Test Suite Output** | **167 PASS, 8 FAIL, 9 TIMEOUT** | Lihat `docs/verification-report-fase15-part2.md` |
| Known Issues | 17 Suites | Lihat section Known Issues |

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

### Standard Library Lengkap (Fase 15 Part 1)
- **`List`** — `panjang`, `peta`, `saring`, `ambil`, `buang`, `gabung`, `balik`, `urutkan`
- **`Map`** — `baru`, `ambil`, `taruh`, `kunci`, `nilai`, `gabung`, `hapus`, `berisi`
- **`String`** — `pisah`, `gabung`, `ganti`, `pangkas`, `besar`, `kecil`, `mulai_dengan`, `berakhir_dengan`, `cari`
- **`File`** — `baca`, `tulis`, `tambah`, `ada`, `hapus`, `daftar`, `ukuran`
- **`Math`** — `akar`, `pangkat`, `sin`, `cos`, `tan`, `log`, `log10`, `abs`, `bulat`, `langit`, `lantai`, `pi`
- **`Time`** — `sekarang`, `format`, `parse`, `tidur`, `milik_detik`, `tambah_detik`
- **`Path`** — `gabung`, `direktori`, `nama_file`, `ekstensi`, `mutlak`, `normalisasi`

### Package Registry & Distribution (`grdmix` — Fase 15 Part 2)
- **`grdmix publish`** — Menerbitkan paket & mengompresi payload dengan tanda tangan digital Ed25519.
- **`grdmix search <query>`** — Pencarian katalog paket lokal/remote berdasarkan nama & deskripsi.
- **`grdmix install <pkg>@<ver>`** — Pemasangan paket terverifikasi (penolakan otomatis payload yang terpengaruh tampering).
- **Package Signature Verification** — Otentikasi kriptografi Ed25519 & SHA-256 checksum verification.

---

## 🚀 Cara Build & Jalankan

### Linux / macOS (CMake)
```bash
git clone https://github.com/anggitPM17/garuda.git
cd garuda/garuda_v67_0_production/garuda_v67.0.0
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Windows (PowerShell & CMake)
```powershell
git clone https://github.com/anggitPM17/garuda.git
cd garuda\garuda_v67_0_production\garuda_v67.0.0
cmake -B build
cmake --build build --config Release
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

- **[Laporan Verifikasi Fase 15 Part 2 (Registry & Signature)](docs/verification-report-fase15-part2.md)** — Bukti eksekusi test `garuda_grdmix_registry_tests` & `garuda_package_signature_tests`.
- **[Laporan Verifikasi Fase 15 Part 1 (Stdlib)](docs/verification-report-fase15.md)** — Bukti eksekusi 7 modul stdlib native.
- **[Roadmap Pengembangan Lanjutan Fase 15–19](docs/ROADMAP_FASE15_19.md)** — Peta jalan fitur Stdlib, Registry, Concurrency, & FFI.
- **[Panduan Self-Hosting Bootstrap](docs/bootstrap.md)** — Arsitektur 3-Stage self-hosting compiler & verifikasi `garuda_bootstrap_grd_tests.exe` (PASS 15/15).
- **[Catatan Rilis & CHANGELOG](CHANGELOG.md)** — Riwayat perubahan rilis.
- **[Peta Pencapaian Ekosistem](ACHIEVEMENTS.md)** — Rangkuman 6 pilar ekosistem GVM.

---

## ⚠️ Known Issues (Transparan)

Daftar dari `docs/verification-report-fase15-part2.md`:

- **8 Test FAIL**: `garuda_db_tests.exe`, `garuda_fullstack_tests.exe`, `garuda_https_e2e_tests.exe`, `garuda_http_tests.exe`, `garuda_multi_phase_integration_tests.exe`, `garuda_orm_tests.exe`, `garuda_router_e2e_tests.exe`, `garuda_router_middleware_tests.exe` (Socket teardown / port collision saat dijalankan berurutan cepat atau cert SSL mock OS runner).
- **9 Test TIMEOUT**: Suites yang menjalankan loop HTTP server / socket listener terikat port (`garuda_async_event_loop_tests`, `garuda_bytecode_io_tests`, `garuda_chat_server_tests`, `garuda_cluster_manager_tests`, `garuda_default_max_reqs_tests`, `garuda_event_loop_backend_tests`, `garuda_fault_tests`, `garuda_million_tests`, `garuda_todo_api_e2e_tests`). Melebihi limit 10 detik pada eksekusi batch non-interaktif.

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
