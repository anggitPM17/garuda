# Garuda Programming Language (GVM) 🇮🇩

**Garuda** adalah bahasa pemrograman terdistribusi modern *general-purpose* yang ditulis 100% oleh anak Indonesia, berbasis **Sintaks Indonesia + Model Aktor Konkuren** (terinspirasi BEAM / Erlang / Elixir). 

Garuda v67.0.0 hadir secara berdikari dengan dukungan lintas platform: **Backend Microservices, Mobile Native (Android/iOS), WebAssembly (Browser), hingga Game Engine 2D/3D & Full Self-Hosting Compiler**.

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache--2.0-yellow.svg)](LICENSE)
[![Made in Indonesia](https://img.shields.io/badge/Made%20in-Indonesia-red)](https://richat.my.id)
[![Version](https://img.shields.io/badge/version-v67.0.0--final--release-blue)](CHANGELOG.md)

---

## 🚀 Status Proyek (v67.0.0 Final Production Release)

> **Rilis Resmi:** `v67.0.0-final-release`  
> **Laporan Resmi:** [`docs/FINAL_RELEASE_REPORT.md`](docs/FINAL_RELEASE_REPORT.md)  
> **Distribusi Biner:** [`dist/garuda_v67.0.0_windows_x64.zip`](dist/garuda_v67.0.0_windows_x64.zip)

| Lapis Arsitektur | Modul Utama | Status Uji |
|---|---|---|
| **Lapis 0 (Fondasi Bahasa & VM)** | Interpreter Biner GVM, TCO O(1), Preemption, Hybrid Memory Allocator, Stdlib. | ✅ **PASS (100%)** |
| **Lapis 1 (BEAM & Server Layer)** | Actor Model, Mnesia/Jalur Langit DB, WebRTC, Autopilot AI, Telemetry, RICHAT Server. | ✅ **PASS (100%)** |
| **Lapis 2 (General Purpose & Enterprise)** | Standalone AOT Packer (`grdc --standalone`), Script Sandboxing (`GvmSandbox`), Embedding API, PQC Hybrid (ML-KEM-768). | ✅ **PASS (100%)** |
| **Lapis 3 (Mobile Engine)** | Android NDK Asset Bundler (`assets/app.grdbc`), Mobile Lifecycle State Machine, Native Bridge UI Binding, Mobile Offline Store. | ✅ **PASS (100%)** |
| **Self-Hosting Compiler** | Full Self-Hosting Compiler (`grdc.grd`: Lexer, Parser, CodeGen) via *3-Stage Bootstrapping*. | ✅ **PASS (100%)** |
| **Lapis 4 (Game Engine Platform)** | Pure Garuda Scripting: ECS Core, 3D Scene Graph, 3D Physics Engine, 3D Spatial Audio. | ✅ **PASS (100%)** |
| **Peranti Lunak GUI** | **`EditorGaruda.exe` v3.5**: Native Windows GUI Editor & **`GarudaStudioIDE`**: Auto-Grader Training Engine. | ✅ **PASS (100%)** |

---

## 📦 Fitur Kunci Ekosistem Garuda

### 🦅 1. Full Self-Hosting Compiler (`grdc.grd`)
- **Self-Hosting Lexer (`packages/compiler/src/lexer.grd`)**: Tokenizer murni Bahasa Garuda.
- **Self-Hosting Parser (`packages/compiler/src/parser.grd`)**: Recursive Descent Parser murni Bahasa Garuda.
- **Self-Hosting CodeGen (`packages/compiler/src/codegen.grd`)**: Opcode Generator murni Bahasa Garuda.
- **3-Stage Bootstrapping**: Stage 0 C++ Compiler mengompilasi Stage 1 Pure Garuda Compiler.

### ⚙️ 2. General-Purpose & Enterprise Integration (Lapis 2)
- **`grdc --standalone <out.exe>`**: Mengkompilasi berkas `.grd` langsung menjadi satu file biner `.exe` mandiri tanpa dependensi external runtime.
- **`GvmSandbox`**: Pembatasan hak akses I/O, Filesystem, Jaringan, Exec, dan RAM per-thread/per-actor.
- **`GvmEmbeddingHost`**: C++ Host Integration API agar GVM dapat ditanamkan pada aplikasi C++/Game Engine lain.
- **Post-Quantum Cryptography**: Pertukaran kunci hybrid NIST FIPS 203 **ML-KEM-768 Kyber** + ECDH P-256.

### 📱 3. Mobile Engine Android & iOS (Lapis 3)
- **`GvmAndroidPacker`**: Pembundelan AOT Bytecode + metadata mobile (`app_id`, `target_abi`) ke `assets/app.grdbc`.
- **`GvmMobileLifecycleManager`**: State machine penanganan siklus hidup OS & penghematan daya baterai saat di-background.
- **`GvmNativeBridge`**: Jembatan pesan dua arah antara Aktor GVM (C++) dan UI Host (Kotlin Jetpack Compose / Swift SwiftUI).
- **`GvmMobileStore`**: Penyimpanan lokal terenkripsi & *offline action queue* untuk arsitektur *offline-first*.

### 🎮 4. Game Engine Platform 2D/3D (Lapis 4)
- **Garuda Pure ECS Core (`packages/game/src/ecs.grd`)**: Arsitektur Entity Component System murni Garuda.
- **3D Scene Graph (`packages/game/src/scene3d.grd`)**: Hierarki spasial `SceneNode` dan perkalian matriks *world transform*.
- **3D Physics Engine (`packages/game/src/physics3d.grd`)**: AABB/Sphere collision detection & gravitasi.
- **3D Spatial Audio (`packages/game/src/audio3d.grd`)**: Suara spasial 3D (*listener vs emitter*) & peredaman jarak.

---

## 🛠️ Cara Build & Penggunaan Cepat

### Build Proyek (Windows / Linux)
```cmd
git clone https://github.com/anggitPM17/garuda.git
cd garuda/garuda_v67_0_production/garuda_v67.0.0
cmake -B build
cmake --build build --config Release
```

### Jalankan Skrip Garuda
```cmd
build\Release\grdc.exe run examples/todo_api/todo_api.grd
```

### Kompilasi AOT ke Executable Mandiri (.exe)
```cmd
build\Release\grdc.exe compile contoh.grd --standalone contoh_app.exe
contoh_app.exe
```

### Menjalankan Editor GUI Native Windows
```cmd
build\Release\EditorGaruda.exe
```

---

## 📊 Benchmark Performa (VPS Ubuntu 1GB RAM)

| Metrik | Erlang/OTP | Garuda BEAM C++ | Efisiensi |
|---|---|---|---|
| RAM Idle | ~69 MB | **~536 KB** | **38x Lebih Hemat** |
| Requests / Sec (RPS) | ~50 req/s | **191 req/s** | **3.8x Lebih Cepat** |
| Availability (Load Test) | 99.2% | **100.00%** | **Zero Error** |
| Chaos Recovery (SIGKILL) | ~10 detik | **< 5 detik** | **2x Lebih Tangguh** |

---

## 📚 Dokumentasi

- **[Laporan Resmi Rilis Final (FINAL_RELEASE_REPORT.md)](docs/FINAL_RELEASE_REPORT.md)** — Laporan komprehensif rilis produksi v67.0.0.
- **[Changelog Proyek (CHANGELOG.md)](CHANGELOG.md)** — Catatan riwayat versi dan fitur.
- **[Kebijakan Keamanan (SECURITY.md)](docs/SECURITY.md)** — Audit keamanan, hardening, dan pelaporan kerentanan.

---

🌐 **Portal Resmi**: [https://richat.my.id/](https://richat.my.id/)  
🐙 **GitHub**: [https://github.com/anggitPM17/garuda](https://github.com/anggitPM17/garuda)
