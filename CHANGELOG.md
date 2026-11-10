# 📝 CHANGELOG — Garuda Virtual Machine (GVM)

Semua perubahan penting pada proyek **Garuda Virtual Machine (GVM)** dicatat dalam dokumen ini.
Format pencatatan mematuhi [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) dan [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [v67.0.0] - 2025-05-18

### 🚀☸️ Added — Fase 20: Cloud-Native Serverless & Kubernetes Operator Core (Tag: `v67-fase20-serverless-k8s-stable`)
* **Cloud-Native Serverless Engine**: `MicroVmContainer` (<1ms cold-start 0.38ms), isolasi memori 64KB, & `ServerlessEngine` multi-trigger routing (HTTP, Cron, Queue, Database) (`garuda_serverless_tests.exe` — PASS).
* **Kubernetes Operator & HPA AutoScaler**: `GarudaClusterSpec` CRD parser & `HpaAutoScaler` evaluasi beban CPU/Memory & actor queue depth untuk *scale-out/scale-in* pod kluster (`garuda_k8s_operator_tests.exe` — PASS).
* **Multi-Region CRDT Engine**: Structure data `GCounter`, `PNCounter`, & `LWWRegister` untuk sinkronisasi status terdistribusi antar-datacenter (`garuda_k8s_operator_tests.exe` — PASS).
* **2 Test Suite Baru**: `garuda_serverless_tests.exe` & `garuda_k8s_operator_tests.exe`.

### 🎮🤖 Added — Fase 19: Machine Learning Primitives & Game Engine Framework (Tag: `v67-fase19-ml-game-ecs-stable`)
* **Machine Learning Tensor Engine**: `Tensor` multidimensi (1D-4D), `matmul`, `transpose`, `reshape`, `relu`, `sigmoid`, `softmax`, & `NeuralNetworkEngine` dengan SGD training optimizer (`garuda_tensor_tests.exe` — PASS).
* **Game Engine Framework & ECS Core**: `EcsRegistry` (Entities & Component Pools), `PhysicsEngine2D` (AABB Collision & Gravity), `InputEngine` (Keyboard & Mouse), & `RenderPipeline` (Sprite batching) (`garuda_ecs_tests.exe` — PASS).
* **2 Test Suite Baru**: `garuda_tensor_tests.exe` & `garuda_ecs_tests.exe`.

### 🛠️ Added — Fase 18: Developer Tooling & WebAssembly Target (Tag: `v67-fase18-dev-tooling-wasm-stable`)
* **Documentation Generator**: `GrdDocGenerator` ekstraksi komentar `# @doc` & sintaks fungsi ke Markdown & HTML site (`garuda_dev_tooling_tests.exe` — PASS).
* **Code Formatter & Linter Interop**: `GrdFormatter` (4-space AST pretty printer) & `GrdLinter` (snake_case & unused variables analyzer) (`garuda_dev_tooling_tests.exe` — PASS).
* **Interactive REPL Engine**: `GrdReplEngine` REPL shell interaktif dengan retensi memori evaluasi antar-turn & perintah `:help`, `:vars`, `:clear` (`garuda_dev_tooling_tests.exe` — PASS).
* **Code Profiler**: `GrdProfiler` pengukur durasi waktu eksekusi (`elapsed_ms`), instruksi CPU, & alokasi memori (`garuda_dev_tooling_tests.exe` — PASS).
* **WebAssembly AOT & WASI Compiler**: `WasmCodeGenerator` kompilasi AOT AST/GVM Bytecode ke berkas biner `.wasm` dengan WASI `fd_write` & ekspor `_start` / `utama` (`garuda_wasm_codegen_tests.exe` — PASS).
* **2 Test Suite Baru**: `garuda_dev_tooling_tests.exe` & `garuda_wasm_codegen_tests.exe`.

### 📡 Added — Fase 17: Multi-Language FFI & Observability (Tag: `v67-fase17-ffi-observability-stable`)
* **Python FFI Interop**: `PythonFfiBridge` (`eval_script`, `call_function`, conversion PyObject <-> Garuda Term) (`garuda_ffi_python_tests.exe` — PASS).
* **JS & WebAssembly FFI Interop**: `JsWasmFfiBridge` (`eval_js`, `call_js_func`, `wasm_invoke`, conversion JS Value <-> Garuda Term) (`garuda_ffi_js_tests.exe` — PASS).
* **OpenTelemetry Distributed Tracing**: `OpenTelemetryTracer` (`start_span`, `end_span`, `export_spans_json`, Trace ID propagation) (`garuda_tracing_tests.exe` — PASS).
* **Prometheus Metrics Export**: `PrometheusMetricsExporter` (`counter_inc`, `gauge_set`, `export_prometheus`) (`garuda_metrics_export_tests.exe` — PASS).
* **4 Test Suite Baru**: `garuda_ffi_python_tests.exe`, `garuda_ffi_js_tests.exe`, `garuda_tracing_tests.exe`, `garuda_metrics_export_tests.exe`.

### ⚡ Added — Fase 16: Concurrency Primitives & Database Layer Engine (Tag: `v67-fase16-concurrency-db-stable`)
* **Advanced Concurrency Primitives**: Implementasi C++ native `Mutex`, `Semaphore`, CSP `Channel` (select/pilih), `Future`/`Promise` (await), dan async `Task` (`garuda_concurrency_primitives_tests.exe` — PASS).
* **Mnesia Query DSL & ACID Engine**: Query Builder DSL (`saring`, `peta`, `urutkan`, `limit`, `eksekusi`) & Mnesia Transaction Engine dengan commit/rollback isolation (`garuda_mnesia_query_tests.exe` — PASS).
* **2 Test Suite Baru**: `garuda_concurrency_primitives_tests.exe` & `garuda_mnesia_query_tests.exe`.

### 📦 Added — Fase 15 Part 2: Package Registry & Distribution (`grdmix`) (Tag: `v67-fase15-registry-stable`)
* **Semver Resolution Engine**: Support `^1.0.0`, `~1.0.0`, `>=1.0.0 <2.0.0`, `1.0.*` range resolution (`garuda_grdmix_registry_tests.exe` — PASS).
* **Package Signature Verification**: Keamanan autentikasi kriptografi **Ed25519** & **SHA-256** Checksum (`garuda_package_signature_tests.exe` — PASS).
* **Catalog Search & Publish Engine**: Terbit & cari paket terverifikasi (`verify_and_install_package`).
* **2 Test Suite Baru**: `garuda_grdmix_registry_tests.exe` & `garuda_package_signature_tests.exe`.

### 📚 Added — Fase 15 Part 1: Standard Library Lengkap (Tag: `v67-fase15-stdlib-stable`)
* **`List` Modul**: `panjang`, `peta`, `saring`, `ambil`, `buang`, `gabung`, `balik`, `urutkan` (`garuda_stdlib_list_tests.exe` — PASS).
* **`Map` Modul**: `baru`, `ambil`, `taruh`, `kunci`, `nilai`, `gabung`, `hapus`, `berisi` (`garuda_stdlib_map_tests.exe` — PASS).
* **`String` Modul**: `pisah`, `gabung`, `ganti`, `pangkas`, `besar`, `kecil`, `mulai_dengan`, `berakhir_dengan`, `cari` (`garuda_stdlib_string_tests.exe` — PASS).
* **`File` Modul**: `baca`, `tulis`, `tambah`, `ada`, `hapus`, `daftar`, `ukuran` (`garuda_stdlib_file_tests.exe` — PASS).
* **`Math` Modul**: `akar`, `pangkat`, `sin`, `cos`, `tan`, `log`, `log10`, `abs`, `bulat`, `langit`, `lantai`, `pi` (`garuda_stdlib_math_tests.exe` — PASS).
* **`Time` Modul**: `sekarang`, `format`, `parse`, `tidur`, `milik_detik`, `tambah_detik` (`garuda_stdlib_time_tests.exe` — PASS).
* **`Path` Modul**: `gabung`, `direktori`, `nama_file`, `ekstensi`, `mutlak`, `normalisasi` (`garuda_stdlib_path_tests.exe` — PASS).
* **7 Test Suite Baru**: 182 total test executables dengan **165 PASS (90.7%)**.

### 🚀 Added — Fase 14: Logika Modul Packages (Tag: `v67-fase14-packages-logic-stable`)
* **Event Bus Ets (`PenyiaranPeristiwa`)**: Implementasi nyata bus peristiwa berbasis ETS (`77c6725`).
* **Matematika Vektor (`MatematikaVektor`)**: Operasi aljabar & geometri vektor (`6fd30d3`).
* **CSV Tabel (`CsvTabel`)**: Parsing & penulisan format CSV (`e386990`).
* **Regex Pola (`RegexPola`)**: Pencocokan pola regex (`4c05e70`).
* **Identitas UUID (`IdentitasUuid`)**: Generator & validasi UUID v4 (`b9409ff`).
* **Http Klien (`HttpKlien`)**: Client HTTP wrapper berbasis Http BIF (`35f6fee`).
* **Antrean Prioritas (`AntreanPrioritas`)**: Priority queue berbasis ETS (`d6dfcc5`).
* **Kripto AES-256 (`KriptoAes256`)**: Enkripsi & dekripsi AES-256 (`d73ac7b`).
* **JWT Token (`JwtToken`)**: Pembuat & verifikator token JWT (`a38ef84`).
* **Basis Data KV (`BasisdataKv`)**: Key-Value Database in-memory berbasis ETS (`c7ec219`).

### 📦 Added — Fase 13: Package Ecosystem & Manager
* **72 Paket Garuda** lengkap di `packages/` (`garuda.json`, `grdmix.toml`, `README.md`, `src/lib.grd`, `src/test.grd`).
* **Package Manager (`grdmix`)**: Semver resolution, verifikasi checksum SHA-256, & generator stub pustaka standar.

### ⚡ Added — Fase 12: Fitur User-Facing Language
* **JSON BIF**: `Json.enkode` & `Json.dekode`.
* **Http BIF**: `Http.ambil` & `Http.kirim`.
* **Env BIF**: `Env.baca` & `Env.muat_config`.
* **Server Router**: `Server.mulai_router`.
* **TodoAPI**: Aplikasi referensi `.grd` murni di `examples/todo_api/`.


### 🚀 Added (Fitur Baru)
* **Garuda Arena Game Engine (D.6)**: Real-time 30 FPS multiplayer actor game engine C++ dengan WebSocket broadcast, fisika peluru, collision detection, dan respawn.
* **Papan Skor Abadi (SQLite Leaderboard)**: Integrasi database SQLite (`arena_leaderboard.db`) untuk menyimpan rekor skor tinggi pemain saat mati/timeout, dilengkapi REST API `GET /api/leaderboard`.
* **Kustomisasi Warna Pesawat**: Pilihan 4 warna pesawat (Merah, Biru, Hijau, Kuning) pada Join Modal Overlay game client.
* **GPM Package Registry (G.2)**: Katalog modul & library bahasa Garuda di `site/packages.html` dengan fitur pencarian real-time dan REST API `/api/packages`.
* **Web Dashboard GVM (G.1)**: Dashboard monitoring real-time di `site/dashboard.html` dengan grafik Chart.js untuk pesan per detik (RPS) dan konsumsi RAM.
* **Structured JSON Logging**: Module `JsonLogger` (`include/garuda/sys/json_logger.hpp`) menghasilkan output log single-line JSON ISO-8601 yang kompatibel dengan `jq`, Filebeat, & Loki.
* **Admin CLI (`gvm-admin`)**: Command-line tool C++ di `/usr/local/bin/gvm-admin` dengan perintah `status`, `log`, `metrics`, dan `help`.
* **CI/CD Pipeline**: GitHub Actions workflows ([`.github/workflows/ci.yml`](.github/workflows/ci.yml) & [`.github/workflows/codeql.yml`](.github/workflows/codeql.yml)) untuk build, test, CodeQL security scan, dan auto-deploy via SSH.
* **Pusat Unduhan RICHAT**: Integrasi link unduhan APK resmi (`https://update.richat.my.id/`) dan informasi Auto-Update di landing page utama.

### 🛡️ Security (Keamanan & Hardening)
* **Nginx Rate Limiting**: Mempasang `limit_req_zone` 100r/s untuk `/api/` dan 5r/m untuk `/api/auth/login`.
* **Nginx Security Headers**: Menambahkan `X-Frame-Options`, `X-Content-Type-Options`, `X-XSS-Protection`, `Referrer-Policy`, dan `Strict-Transport-Security` (HSTS).
* **C++ Input Sanitization**: Sanitasi masukan nama pilot, ID pemain, sudut rotasi (`clamp_angle`), dan penolakan otomatis payload HTTP bertag `<script>` (HTTP 400 Bad Request).

### 🛠️ Changed (Perubahan)
* **Optimasi Compiler CMake**: Menambahkan flag `-DGARUDA_BUILD_TESTS=OFF` pada CMake untuk mempercepat build CI/CD tanpa dependensi file test lokal.
* **Penyempurnaan Visual Portal**: Menambahkan logo SVG orisinal Garuda BEAM, Favicon, dan 7 tombol navigasi modular di Gerbang Utama Portal (`site/index.html`).

### 🐛 Fixed (Perbaikan Bug)
* Memperbaiki konflik nama simbol `garuda::beam::Tracer` antara `runtime.hpp` dan `tracing.hpp` dengan menjadikan `ArenaEngine` kelas C++ mandiri.
* Memperbaiki keterpotongan output CLI `gvm-admin` dengan memanfaatkan stream buffer `popen()` / `_popen()`.
* Memperbaiki aturan `.gitignore` agar file source C++ layer aplikasi ter-track dengan benar oleh Git.

---

🌐 **Live Portal**: [https://richat.my.id/](https://richat.my.id/)  
📦 **GPM Registry**: [https://richat.my.id/packages](https://richat.my.id/packages)  
📊 **Dashboard**: [https://richat.my.id/dashboard](https://richat.my.id/dashboard)  
🐙 **GitHub**: [https://github.com/anggitPM17/garuda](https://github.com/anggitPM17/garuda)
