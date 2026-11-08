# 📝 CHANGELOG — Garuda Virtual Machine (GVM)

Semua perubahan penting pada proyek **Garuda Virtual Machine (GVM)** dicatat dalam dokumen ini.
Format pencatatan mematuhi [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) dan [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [v67.0.0] - 2025-05-18

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
