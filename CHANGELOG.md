# 📝 CHANGELOG — Garuda Virtual Machine (GVM)

Semua perubahan penting pada proyek **Garuda Virtual Machine (GVM)** dicatat dalam dokumen ini.
Format pencatatan mematuhi [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) dan [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [v67.0.0] - 2026-10-29

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
