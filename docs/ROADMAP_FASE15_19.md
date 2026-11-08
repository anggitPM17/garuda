# 🦅 Ide Pengembangan Bahasa Garuda — Roadmap Lanjutan (Fase 15–19)

> **Dokumen Rencana Masa Depan (Fase 15–19)** — Disusun berdasarkan pencapaian Fase 14 (131 test suite, 72 packages, compiler self-hosting terverifikasi).

---

## 🎯 PRIORITAS TINGGI (Dampak Besar, Kompleksitas Sedang)

### 1. **Fase 15: Standard Library Lengkap (Native Modul & BIF)**
**Dampak**: Pengguna bahasa Garuda mendapat pustaka standar yang matang tanpa perlu stub.

**Modul Inti**:
- **`List`** — `map`, `filter`, `reduce`, `zip`, `take`, `drop`, `flatten`
- **`Map`** — `get`, `put`, `keys`, `values`, `merge`, `has_key`
- **`String`** — `split`, `join`, `replace`, `trim`, `to_upper`, `to_lower`, `contains`
- **`File`** — `baca`, `tulis`, `append`, `exists`, `delete`, `mkdir`
- **`Math`** — `sqrt`, `pow`, `sin`, `cos`, `log`, `abs`, `floor`, `ceil`
- **`Time`** — `sekarang`, `format`, `parse`, `sleep`, `stempel_waktu`
- **`Path`** — `gabung`, `direktori`, `nama_file`, `ekstensi`

**Test Suite Target**:
- `garuda_stdlib_list_tests.exe`
- `garuda_stdlib_map_tests.exe`
- `garuda_stdlib_string_tests.exe`
- `garuda_stdlib_file_tests.exe`
- `garuda_stdlib_math_tests.exe`
- `garuda_stdlib_time_tests.exe`
- `garuda_stdlib_path_tests.exe`

---

### 2. **Fase 15: Package Registry & Distribution (`grdmix`)**
**Dampak**: Menghubungkan 72+ paket ke registry pusat terdistribusi.

**Fitur**:
- `grdmix publish` — Unggah paket ke registry server
- `grdmix search <query>` — Pencarian paket interaktif
- `grdmix install <pkg>@<version>` — Unduh & verifikasi dependensi remote
- Registry Server berbasis G-BEAM C++ Runtime
- Verifikasi Tanda Tangan Paket (SHA-256 + Ed25519 Cryptographic Signature)

**Test Suite Target**:
- `garuda_grdmix_registry_tests.exe`
- `garuda_package_signature_tests.exe`

---

### 3. **Fase 16: Concurrency Primitives Tingkat Lanjut**
**Dampak**: Melengkapi model aktor dengan primitif sinkronisasi tingkat tinggi.

**Fitur**:
- `Mutex` — `kunci()`, `buka()`, `coba_kunci()`
- `Semaphore` — `tunggu()`, `lepas()`
- `Channel` — `kirim()`, `terima()`, `pilih()` (select mechanism)
- `Future` / `Promise` — `await()`, `resolve()`, `reject()`
- `Task` — `spawn()`, `join()`, `cancel()`

**Test Suite Target**: `garuda_concurrency_primitives_tests.exe`

---

## 🎯 PRIORITAS MENENGAH (Dampak Sedang, Kompleksitas Sedang-Tinggi)

### 4. **Fase 16: Database Layer (Mnesia-like Query Engine)**
- Query DSL di `.grd`: `tabel("user") |> filter(fn(u) -> u.umur > 18 end)`
- Automatic Indexing, Transaksi ACID, Replikasi Multi-Node Raft, Migration System.
- **Test Suite Target**: `garuda_mnesia_query_tests.exe`, `garuda_mnesia_transaction_tests.exe`

### 5. **Fase 17: Interoperabilitas Multi-Bahasa (FFI)**
- **Python FFI**, **JS / WASM FFI**, **Rust FFI**, **C++ FFI Direct Binding**.
- **Test Suite Target**: `garuda_ffi_python_tests.exe`, `garuda_ffi_js_tests.exe`

### 6. **Fase 17: Observability & Tracing**
- Integrasi OpenTelemetry, Distributed Tracing (Trace ID propagation antar-aktor), JSON structured logging, CPU/Memory profiler.
- **Test Suite Target**: `garuda_tracing_tests.exe`, `garuda_metrics_export_tests.exe`

---

## 🎯 PRIORITAS RENDAH & EKSPANSI (Dampak Spesifik)

### 7. **Fase 18: Developer Tooling & WASM Target**
- Tooling CLI: `grd fmt`, `grd lint`, `grd doc`, `grd repl`, `grd profile`.
- WebAssembly: Kompilasi AOT ke WASM, WASI, Browser JS interop.
- **Test Suite Target**: `garuda_formatter_tests.exe`, `garuda_wasm_compile_tests.exe`

### 8. **Fase 19: Machine Learning & Game Engine Framework**
- ML Primitives: Tensor multidimensi, Aljabar Linear, Gradient Descent.
- Game Framework: Entity-Component-System (ECS), 2D/3D Physics, Rendering, Input Engine.
- **Test Suite Target**: `garuda_tensor_tests.exe`, `garuda_ecs_tests.exe`

---

## 📊 Matriks Urutan Rencana Rilis

| Prioritas | Fase | Komponen Utama | Estimasi Waktu |
|---|---|---|---|
| **1** | **Fase 15** | Standard Library Lengkap (Native BIF) | 2–3 minggu |
| **2** | **Fase 15** | Package Registry & Distribution (`grdmix`) | 2–3 minggu |
| **3** | **Fase 16** | Concurrency Primitives (Mutex, Channel, Task) | 2 minggu |
| **4** | **Fase 16** | Database Query Engine (Mnesia-like DSL) | 3 minggu |
| **5** | **Fase 17** | FFI Multi-Bahasa (Python, JS, Rust) | 3–4 minggu |
| **6** | **Fase 17** | Observability & Tracing (OpenTelemetry) | 2 minggu |
| **7** | **Fase 18** | Developer Tooling (`fmt`, `lint`, `doc`) & WASM Target | 4 minggu |
| **8** | **Fase 19** | ML Primitives & Game Engine Framework (ECS) | 6+ minggu |
