# 📝 CHANGELOG — Garuda Virtual Machine (GVM)

Semua perubahan penting pada proyek **Garuda Virtual Machine (GVM)** dicatat dalam dokumen ini.
Format pencatatan mematuhi [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) dan [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [v67.0.0] - 2025-05-18

### 🖥️🎓 Added — Interactive Participant Training Hub (`EditorGaruda.exe`)
* **Interactive Training Hub**: Pengayaan biner `EditorGaruda.exe` (`src/editor_garuda_main.cpp`) dengan dukungan pembukaan berkas kodingan peserta via argumen CLI/drag-and-drop, pemuat kurikulum pelatihan interaktif (Modul 1 s.d 4), serta pemindaian visual 389 katalog paket resmi (`EditorGaruda.exe` — PASS).

### 🖥️🎓 Added — GARUDA CODING STUDIO v2.0 & Auto-Grader Training Engine (`GarudaStudioIDE`)
* **Auto-Grader Training Engine**: Penilaian otomatis kodingan peserta pelatihan (`cek_jawaban_peserta`) dengan skor dan umpan balik langsung (`packages/ide/src/garuda_studio.grd`).
* **One-Click Standalone Export**: Ekspor biner `.exe` mandiri satu-klik dari UI Studio (`ekspor_ke_exe_standalone`).
* **Visual GPM Package Installer**: Pemasangan paket dari 389 katalog resmi secara visual (`pasang_paket_gpm`).
* **Inline Linter & Style Suggestions**: Analisis linter otomatis untuk memberikan saran penulisan kode Garuda yang rapi (`tampilkan_linter_dan_saran`) (`garuda_studio_ide_tests.exe` — PASS).

### 🎨📦 Added — High-Level Enrichments: Canvas 2D Vector Drawing, App Installer Creator & Live Demo Showcase
* **GarudaCanvas2D**: Pustaka gambar vektor 2D murni Bahasa Garuda (`packages/gui/src/canvas2d.grd`) (`garuda_canvas2d_tests.exe` — PASS).
* **GvmInstallerCreator**: Mesin pembentuk Windows Setup Installer `.msi` (`include/garuda/compiler/installer_creator.hpp`) (`garuda_installer_creator_tests.exe` — PASS).
* **Live Demo Showcase Script**: Skrip peragaan langsung murni Bahasa Garuda (`examples/live_demo/demo_showcase.grd`) yang memamerkan seluruh 13 kapabilitas ekosistem Garuda.

### 🌐⚡ Added — WebAssembly (Wasm) Web-Runner Engine (`WasmRunnerGaruda`)
* **WasmRunnerGaruda**: Pustaka eksekusi biner Wasm murni Bahasa Garuda (`packages/web/src/wasm_runner.grd`) untuk membundel bytecode ke modul `.wasm` (`buat_modul_wasm`) dan mengalokasikan halaman memori Wasm linear (`alokasi_memori_linier`) di browser (`garuda_web_wasm_runner_tests.exe` — PASS).

### 🧠🤖 Added — On-Device Local AI / LLM Inference Engine (`LlmInferenceGaruda`)
* **LlmInferenceGaruda**: Engine inferensi AI/LLM On-Device murni Bahasa Garuda (`packages/ai/src/llm_inference.grd`) untuk pemuatan model kuantisasi INT8/INT4 (`muat_model_lokal`), BPE tokenisasi prompt (`tokenisasi_prompt`), dan generasi teks autoregresif luring (`generasi_teks`) (`garuda_ai_llm_inference_tests.exe` — PASS).

### 🖥️🎨 Added — Desktop Native GUI Framework (`GarudaUI`)
* **GarudaUI Framework**: Pustaka antarmuka pengguna desktop deklaratif murni Bahasa Garuda (`packages/gui/src/garuda_ui.grd`) untuk membangun aplikasi jendela desktop (`buat_jendela`, `tambah_tombol`, `tambah_teks_input`, `tambah_kotak_pesan`) tanpa konsumsi memori besar Electron (`garuda_ui_framework_tests.exe` — PASS).

### ⚡🔄 Added — Zero-Downtime Hot Code Reloading & Live Beam Code Swap (`GvmLiveHotReloader`)
* **GvmLiveHotReloader**: Mesin pembaruan kode *live* GVM (`include/garuda/beam/gvm/hot_reload_live.hpp`) untuk memperbarui offset instruksi fungsi Aktor secara otomatis tanpa mematikan proses server/game (`garuda_hot_reload_live_tests.exe` — PASS).

### 🎨💻 Added — Official VS Code Extension & Language Server Protocol (LSP) Engine (`garuda-vscode-lsp`)
* **TextMate Syntax Highlighting**: Grammar resmi `vscode-extension/syntaxes/garuda.tmLanguage.json` dengan pewarnaan kata kunci Indonesia (`modul`, `fungsi`, `tampa`, `kirim`, `cetak`, `yen`, `lain`, `coba`, `tangkap`, `rampung`), atom (`:ok`), string, angka, dan komentar `#`.
* **Language Configuration**: Auto-closing brackets `()`, `[]`, `{}` dan aturan auto-indentasi otomatis (`vscode-extension/language-configuration.json`).
* **LSP Stdlib IntelliSense & Hover Tooltips**: Auto-completion dan Markdown hover documentation (`include/garuda/compiler/lsp_engine.hpp`) untuk seluruh Pustaka Standar (`String`, `List`, `Map`, `File`, `Math`, `Time`, `Path`) (`garuda_lsp_tests.exe` — ALL 10 PASS).

### 📦🚀 Added — Integrated Package Manager Client (`grdc pkg`) & 389 Packages Audit
* **Integrated CLI Command**: Perintah CLI `grdc pkg list` (mendaftar seluruh 389 paket terverifikasi), `grdc pkg verify` (verifikasi integritas 389 paket), dan `grdc pkg install <nama_paket>` (memasang dependensi ke direktori proyek) (`garuda_package_manager_client_tests.exe` — PASS).
* **Package Manager Engine**: `GpmPackageManager` (`include/garuda/pkg/package_manager.hpp`) untuk pindaian manifes JSON/TOML, verifikasi struktur paket `src/lib.grd`, dan pengelolaan instalasi lokal.

### 🎮🕹️ Added — LAPIS 4: Game Engine Platform & Real-Time Graphics Engine (Pure Garuda Script Engine)
* **Stage 4.1 (ECS Core)**: Modul `GameEcsGaruda` (`packages/game/src/ecs.grd`) untuk manajemen dunia game (`buat_dunia`, `buat_entitas`), alokasi komponen data (`:posisi`, `:kecepatan`), dan sistem pergerakan (`sistem_gerak_update`) (`garuda_game_ecs_tests.exe` — PASS).
* **Stage 4.2 (3D Scene Graph & PBR)**: Modul `GameScene3DGaruda` (`packages/game/src/scene3d.grd`) untuk hierarki spasial 3D (`buat_node_3d`, `tambah_anak_node`), kalkulasi matriks world transform, dan PBR Material pipeline (`buat_material_pbr`) (`garuda_game_scene3d_tests.exe` — PASS).
* **Stage 4.3 (3D Physics Engine)**: Modul `GamePhysics3DGaruda` (`packages/game/src/physics3d.grd`) untuk AABB 3D collision detection, gravitasi, dan update simulasi fisika 3D (`garuda_game_physics3d_tests.exe` — PASS).
* **Stage 4.4 (3D Spatial Audio)**: Modul `GameAudio3DGaruda` (`packages/game/src/audio3d.grd`) untuk posisi pendengar vs sumber suara dan kalkulasi peredaman jarak 3D (*inverse distance attenuation*) (`garuda_game_audio3d_tests.exe` — PASS).

### 🦅🔥 Added — Full Self-Hosting Compiler Bahasa Garuda (`grdc.grd`) (3-Stage Bootstrapping)
* **Self-Hosting Lexer**: Tokenizer murni Bahasa Garuda (`packages/compiler/src/lexer.grd`) yang memecah skrip `.grd` menjadi tuple token (`garuda_self_hosting_lexer_tests.exe` — PASS).
* **Self-Hosting Parser**: Recursive Descent Parser murni Bahasa Garuda (`packages/compiler/src/parser.grd`) yang membangun Pohon Sintaksis Abstrak (AST) (`garuda_self_hosting_parser_tests.exe` — PASS).
* **Self-Hosting CodeGen**: Opcode Generator murni Bahasa Garuda (`packages/compiler/src/codegen.grd`) yang menerjemahkan AST ke instruksi biner GVM (`garuda_self_hosting_codegen_tests.exe` — PASS).
* **3-Stage Bootstrapping**: Transisi sukses di mana Compiler C++ Stage 0 (`grdc.exe`) mengompilasi Compiler Murni Garuda (`examples/compiler/grdc.grd`) (`garuda_self_hosting_bootstrap_tests.exe` — PASS).

### 💾📡 Added — Lapis 3 Tahap 3.4: Mobile Offline Store & Resilience Sync Engine (`GvmMobileStore`)
* **GvmMobileStore**: Mesin penyimpanan lokal terenkripsi dan antrean aksi luring (*offline action queue*) (`include/garuda/mobile/mobile_store.hpp`) untuk arsitektur mobile *offline-first* dengan sinkronisasi otomatis saat terhubung kembali (`garuda_mobile_store_tests.exe` — PASS).

### 🌉📱 Added — Lapis 3 Tahap 3.3: Native Bridge & Host UI Binding Interface (`GvmNativeBridge`)
* **GvmNativeBridge**: Jembatan pesan dua arah (`include/garuda/mobile/native_bridge.hpp`) antara Runtime Aktor GVM Garuda (C++) dengan UI Host Native (Kotlin Jetpack Compose / Swift SwiftUI) tanpa perlu membuat UI engine dari nol (`garuda_native_bridge_tests.exe` — PASS).

### 🔄🔋 Added — Lapis 3 Tahap 3.2: Mobile Lifecycle & Battery State Machine (`GvmMobileLifecycleManager`)
* **GvmMobileLifecycleManager**: State machine penanganan siklus hidup OS Android/iOS (`include/garuda/mobile/mobile_lifecycle.hpp`) dengan transisi status (`CREATED`, `STARTED`, `RESUMED`, `PAUSED`, `STOPPED`, `DESTROYED`) (`garuda_mobile_lifecycle_tests.exe` — PASS).
* **Power Saving & Memory Saver Hooks**: Otomatisasi pengaktifan *Power-Saving Mode* pada Aktor GVM saat `ON_PAUSE`/`ON_STOP` dan callback pelepasan memori pada event `ON_TRIM_MEMORY`.

### 📱🤖 Added — Lapis 3 Tahap 3.1: Android NDK Cross-Compiler & APK Asset Bundler (`GvmAndroidPacker`)
* **GvmAndroidPacker**: `GvmAndroidPacker` & `AndroidPackerConfig` (`include/garuda/mobile/android_packer.hpp`) membundel AOT Bytecode Garuda + header metadata mobile (`app_id`, `target_abi`, `version_code`, `version_name`) + FNV-1a checksum menjadi berkas aset APK `assets/app.grdbc` (`garuda_android_packer_tests.exe` — PASS).
* **Integrasi NDK Runtime**: Ekstraksi dan verifikasi integritas bytecode di memori NDK Android saat inisialisasi `JNI_OnLoad`.

### 🔐🔑 Added — Lapis 2 Tahap 4: Post-Quantum Cryptography Hybrid Layer (ML-KEM-768 Kyber)
* **PqcHybridEngine**: Implementasi NIST FIPS 203 ML-KEM-768 Kyber Key Encapsulation Mechanism (`include/garuda/beam/crypto/pqc_hybrid.hpp`) untuk pertukaran kunci kriptografi resisten-kuantum (`garuda_pqc_hybrid_tests.exe` — PASS).
* **Hybrid Secret Derivation**: Penggabungan kunci klasik ECDH P-256 dan ML-KEM-768 Kyber menjadi kunci master hybrid 32-byte.

### 🔌💻 Added — Lapis 2 Tahap 3: Interop C++ Embedding Host API (`GvmEmbeddingHost`)
* **GvmEmbeddingHost API**: Host Integration Engine (`include/garuda/beam/interop/embedding_api.hpp`) yang memungkinkan aplikasi C++/game engine menanamkan runtime GVM (kompilasi skrip, penetapan variabel global, dan pemanggilan fungsi) secara langsung (`garuda_embedding_api_tests.exe` — PASS).

### 🛡️🔒 Added — Lapis 2 Tahap 2: Script Sandboxing & Resource Isolation Engine (`GvmSandbox`)
* **GvmSandbox Engine**: `GvmSandbox` & `SandboxPolicy` (`include/garuda/beam/sys/sandbox.hpp`) menyediakan isolasi keamanan per-thread/per-actor untuk membatasi hak akses Filesystem (Read/Write), Jaringan (Socket Connection), Subprocess Execution, dan Memori Maksimal (`garuda_sandbox_tests.exe` — PASS).
* **Path Whitelisting**: Dukungan daftar kustom `allowed_paths` untuk mengizinkan akses I/O khusus ke folder/file tertentu dalam mode terisolasi.

### 📦⚙️ Added — Lapis 2 Tahap 1: Standalone AOT Binary Compiler & Packer (`grdc --standalone`)
* **Standalone AOT Packer**: `StandalonePacker` (`include/garuda/compiler/standalone_packer.hpp`) membundel biner stub runner + bytecode `.grdbc` + 28-byte footer magic (`GARUDA_STUB_V1_FOOTER`) menjadi berkas biner `.exe` mandiri (`garuda_standalone_packer_tests.exe` — PASS).
* **CLI Compiler Extension**: Opsi `grdc compile <file.grd> --standalone <output.exe>` untuk mengkompilasi file `.grd` langsung menjadi aplikasi biner mandiri tanpa dependensi runtime eksternal.
* **Auto-Execution Detection**: Auto-detect payload bytecode pada saat eksekusi langsung biner `.exe` terbundel.

### 🌳🛠️ Added — Fase 23: Modular CMake Architecture, 3D Scene Graph Engine & Structured Test Runner (Tag: `v67-fase23-scenegraph-cmake-stable`)
* **3D Scene Graph Engine**: `SceneNode` 3D spatial hierarchy dengan world transform matrix computation, visitor traversal, & automatic scene rendering pass (`garuda_scene_graph_tests.exe` — PASS).
* **Modular CMake Automation & Test Runner**: Skrip Python `tools/cmake_helper.py` teruji untuk registrasi CMake target otomatis & runner PowerShell dengan UTF-8 console encoding + progress logging interaktif + summary JSON export (`garuda_cmake_structure_tests.exe` — PASS).
* **3 Test Suite Baru & Bug Fixes**: `garuda_scene_graph_tests.exe`, `garuda_cmake_structure_tests.exe`, & `garuda_frustum_culling_tests.exe` — seluruh 16 sub-test `garuda_tests.exe` kini **PASS 100%**.

### 🎮🔊 Added — Fase 22: High-Performance 3D Render Engine & 3D Physics/Spatial Audio (Tag: `v67-fase22-render3d-physics3d-stable`)
* **Native 3D Render Engine**: `Mesh3D`, `Material3D` PBR, `Camera3D`, & `Render3DPipeline` deferred rendering pass dengan multi-light support (`garuda_render3d_tests.exe` — PASS).
* **3D Physics Engine & Spatial Audio 3D**: `PhysicsEngine3D` AABB 3D collision & gravity integration, serta `SpatialAudio3D` inverse distance attenuation curve (`garuda_physics3d_tests.exe` — PASS).
* **2 Test Suite Baru**: `garuda_render3d_tests.exe` & `garuda_physics3d_tests.exe`.

### ⚡🧠 Added — Fase 21: Deep Learning GPU Acceleration & ONNX Model Importer (Tag: `v67-fase21-gpu-onnx-stable`)
* **GPU Accelerator Engine**: `GpuDeviceManager` (CUDA/Metal/Vulkan/ROCm/SIMD) & `GpuTensor` alokasi memori VRAM & kernel paralel `gpu_matmul`, `gpu_add`, `gpu_relu` (`garuda_gpu_accelerator_tests.exe` — PASS).
* **ONNX & Safetensors Model Importer**: `OnnxModelLoader` graph parser & `OnnxInferenceEngine` eksekusi inferensi AI langsung pada GPU (`garuda_onnx_loader_tests.exe` — PASS).
* **2 Test Suite Baru**: `garuda_gpu_accelerator_tests.exe` & `garuda_onnx_loader_tests.exe`.

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
