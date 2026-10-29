# 🦅 GVM — Garuda Virtual Machine

> **Runtime Terdistribusi C++ Setara Erlang/OTP. 100% Karya Anak Bangsa 🇮🇩**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-C%2B%2B17%20%7C%20Android%20NDK%20%7C%20WASM-blue)](https://richat.my.id)
[![Production Status](https://img.shields.io/badge/Production-Live%20ONLINE-22c55e)](https://richat.my.id)
[![RAM Usage](https://img.shields.io/badge/RAM-536%20KB%20--%201.8%20MB-brightgreen)](https://richat.my.id)
[![Performance](https://img.shields.io/badge/RPS-3.8x%20vs%20Erlang-orange)](https://richat.my.id)

---

<p align="center">
  <img src="site/assets/garuda-logo.svg" alt="Garuda Logo" width="140" height="140">
</p>

## 🌐 Ringkasan Proyek

**Garuda Virtual Machine (GVM)** adalah *distributed actor-model runtime* C++17 berkinerja tinggi yang dirancang sebagai alternatif **Erlang/OTP** yang jauh lebih hemat memori dan berlatensi ultra-rendah. 

GVM dibangun tanpa dependensi berat, mengusung arsitektur *header-only*, serta mendukung eksekusi terdistribusi di server Linux, WebAssembly (WASM) di browser, hingga C++ Native Layer pada aplikasi **RICHAT (Android)**.

Portal Resmi & Live Production aktif di **[https://richat.my.id/](https://richat.my.id/)**.

---

## 🏛️ Ekosistem Portal Utama (Live Demo)

Portal utama di **[richat.my.id](https://richat.my.id/)** menyediakan 7 modul ekosistem terpadu:

| Destinasi Modul | Deskripsi & Kegunaan | URL Akses |
| :--- | :--- | :--- |
| 🏟️ **Stadion Garuda** | Interactive Web Playground untuk mengeksekusi kode Garuda. | [Masuk Stadion](https://richat.my.id/stadion/) |
| 🌐 **WASM Client** | Execution Environment berbasis WebAssembly langsung di browser. | [Buka WASM](https://richat.my.id/playground_wasm) |
| 🎮 **Garuda Arena** | Game multiplayer real-time 30 FPS berbasis Actor Model & SQLite Leaderboard. | [Mainkan Game](https://richat.my.id/assets/game.html) |
| 📊 **Dashboard GVM** | Dashboard monitoring Prometheus metrics, RAM/CPU, dan RPS real-time. | [Buka Dashboard](https://richat.my.id/dashboard) |
| 📦 **GPM Packages** | Katalog pustaka resmi & modul package manager bahasa Garuda. | [Lihat Packages](https://richat.my.id/packages) |
| 🐙 **Repository** | Kode sumber resmi di GitHub. | [GitHub Repo](https://github.com/anggitPM17/garuda) |
| 📱 **Unduh RICHAT** | Aplikasi Android resmi dengan fitur Auto-Update terpadu. | [Unduh APK](https://update.richat.my.id/) |

---

## ✨ Fitur Utama Runtime

* **⚡ OTP Behaviours**: Implementasi lengkap `GenServer`, `Supervisor` trees, `Application`, `PubSub`, dan `GenStatem` state machines.
* **🌐 Cluster Terdistribusi**: `DistNode`, `GlobalRegistry` berbasis Raft consensus, `ClusterManager`, dan `ProcessGroup (PG)`.
* **🗄️ Storage Mnesia & SQLite**: In-memory & DiscCopies Set/Bag tables, Tiered Storage (Hot/Warm/Cold), TTL Sweeper, dan integrasi SQLite WAL.
* **🔒 End-to-End Cryptography**: AES-GCM 256, X25519 DH, Key Transparency Merkle Tree, Sealed Sender, & JWT Auth.
* **🎮 Garuda Arena Game Engine**: Real-time 30 FPS multiplayer game engine berbasis Actor Model dengan Papan Skor Abadi SQLite.
* **📊 Live Monitoring Dashboard**: Web Dashboard visualisasi Prometheus metrics dengan grafik pergerakan pesan dan konsumsi RAM real-time.
* **📦 GPM Package Manager**: Registry katalog package resmi untuk modul bahasa Garuda.
* **📱 Android NDK Engine**: C++ Native Layer untuk sinkronisasi offline dan komunikasi real-time pada aplikasi RICHAT.

---

## 📊 Benchmark Performa Produksi (VPS Ubuntu 1GB RAM)

| Metrik Performa | Erlang/OTP Server | Garuda BEAM C++ Engine | Tingkat Efisiensi |
| :--- | :---: | :---: | :---: |
| **RAM Usage (Idle)** | ~69.4 MB - 92.8 MB | **~536 KB - 1.8 MB** | **38x - 50x Lebih Hemat** |
| **Requests / Sec (RPS)** | ~50 req/s | **191.65 req/s** | **3.8x Lebih Cepat** |
| **Availability (Load Test)** | 99.2% | **100.00% (0 Failed)** | **100% Zero Error** |
| **Chaos Recovery (SIGKILL)** | ~10 detik | **< 5 detik** | **2x Lebih Tangguh** |

---

## 📐 Arsitektur Sistem

```
                 +-----------------------------------+
                 |     Android RICHAT App (Kotlin)   |
                 |     + C++ Native Layer (NDK)      |
                 +-----------------+-----------------+
                                   | WebSocket (WSS) / REST
                                   v
+----------------------------------+----------------------------------+
|                   NGINX Reverse Proxy (HTTPS/WSS)                   |
|                      Domain: richat.my.id                           |
+----+--------------------+-------------------+-------------------+---+
     |                    |                   |                   |
     v                    v                   v                   v
/dashboard            /packages          /assets/game.html       /api/ & /ws
  (HTML5/Chart.js)      (GPM Catalogue)    (HTML5 Canvas)       (Port 9002)
     |                    |                   |                   |
     +--------------------+-------------------+-------------------+
                                   |
                                   v
                 +-----------------------------------+
                 |    Garuda BEAM C++ Server (GVM)   |
                 |  - OTP Scheduler & GenServer      |
                 |  - Garuda Arena Game Engine       |
                 |  - GPM Registry & SQLite DB       |
                 +-----------------------------------+
```

---

## 🚀 Cara Build & Menjalankan

### Linux / macOS (CMake & Make)
```bash
# 1. Clone repository
git clone https://github.com/anggitPM17/garuda.git
cd garuda/garuda_v67_0_production/garuda_v67.0.0

# 2. Build via CMake
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make garuda_richat_server -j$(nproc)

# 3. Jalankan server
./garuda_richat_server 9002
```

### Windows (PowerShell & CMake)
```powershell
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\garuda_richat_server.exe 9002
```

### Docker Container
```bash
docker build -t garuda-beam .
docker run --rm -p 9002:9002 garuda-beam
```

---

## 📚 Dokumentasi & Catatan Rilis

* 🏆 **[Peta Pencapaian Ekosistem (ACHIEVEMENTS.md)](ACHIEVEMENTS.md)** — Rangkuman 6 pilar ekosistem GVM.
* 📝 **[Catatan Rilis & History (CHANGELOG.md)](CHANGELOG.md)** — History rilis v67.0.0 dan catatan perubahan.
* 🔑 **[Panduan Deployment CI/CD (README_DEPLOY.md)](README_DEPLOY.md)** — Setup GitHub Secrets & SSH Deploy Key.

---

## 📜 Lisensi & Kontribusi

Proyek ini dilesensikan di bawah **[MIT License](LICENSE)** — bebas digunakan, dimodifikasi, dan didistribusikan untuk keperluan personal maupun komersial.

Kontribusi sangat disambut baik! Silakan ajukan *Issue* atau *Pull Request* di repositori resmi **[GitHub anggitPM17/garuda](https://github.com/anggitPM17/garuda)**.

---
**Dibuat dengan ❤️ di Indonesia. &copy; 2026 Garuda BEAM Contributors. 🇮🇩**
