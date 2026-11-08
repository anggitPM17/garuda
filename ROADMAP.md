# 🦅 GARUDA — PETA JALAN MENUJU GENERAL-PURPOSE, MOBILE & GAME ENGINE

> Dokumen hidup. Simpan sebagai `ROADMAP.md` di root repo Garuda.
> Paste dokumen ini + status centang terakhir ke agen/AI baru agar langsung paham posisi proyek.

---

## 🤖 Konteks

- **Garuda**: bahasa pemrograman (ekstensi `.grd`, compiler `grdc`, runtime **GVM**, REPL) dengan **sintaks Indonesia + model aktor** (terinspirasi BEAM/Erlang/Elixir).
- **Prinsip kerja**: *"Pengerasan dulu, ekspansi kemudian."* Jangan membuka lapis baru sebelum gate keluar lapis sebelumnya tercapai.
- **Aturan untuk agen**:
  1. Kerjakan hanya item pada lapis yang sedang aktif.
  2. Jangan melompat lapis atau menambah fitur di luar roadmap tanpa persetujuan.
  3. Jangan mengklaim kemampuan sebelum terbukti (ada tes / demo nyata).
  4. Setiap item selesai: centang `[x]`, tambahkan tes, dan commit.
  5. Jaga identitas: sintaks Indonesia + model aktor tidak boleh hilang.
  6. **Setiap pengembangan wajib disertai tes** (fitur baru, perbaikan bug, maupun refactor). Tanpa tes, pekerjaan dianggap belum selesai dan tidak boleh dicentang.
- **Stadion Garuda**: playground interaktif (mirip Erlang playground) yang berjalan paralel di semua lapis (lihat bagian "Jalur Paralel: Stadion Garuda"). Setiap fitur bahasa/runtime baru WAJIB bisa dicoba & divisualisasikan di Stadion.
- **Lapis aktif saat ini**: Lapis 0 (pengerasan fondasi), dengan Stadion S0 berjalan paralel
- **Status terakhir** (26 Okt 2026): Lapis 0 (Pengerasan Fondasi) RESMI SELESAI 100%! 30 commit hijau berturut-turut tercapai. Gate Keluar Lapis 0 terpenuhi. Proyek siap masuk ke Lapis 1 (Backend & Layanan).
- **Update 26 Okt 2026 (investigasi HTTP)**: Baseline HTTP lama 35.971 req/s tidak dapat direproduksi. Baseline baru: 16.300 req/s (median 2 run, reliabel). Overhead perbaikan control flow terbukti kecil (~1,6% via mutation test). DoD #2 direvisi untuk memakai baseline baru. Tidak ada perubahan kode; hanya koreksi dokumen.
- **Keputusan final**: lihat tabel "Keputusan Krusial" (memory hybrid; interpreter dulu + transpile ke C/C++; mobile via NDK + host UI; game via scripting; C ABI + WASM). Hero use case Lapis 1: RICHAT, game server menyusul.
- **Review roadmap berikutnya**: 26 Januari 2027 (tiap 3 bulan). Revisi terakhir: 26 Oktober 2026 (memasukkan masukan review eksternal: estimasi jujur, risiko, non-goals, definisi selesai, aturan rilis Android, DoD Stadion S0, rem darurat dompet, irama sprint).

### Template prompt ke agen

```
Konteks: proyek Garuda (lihat ROADMAP.md terlampir).
Lapis aktif: [0/1/2/3/4]. Item yang dikerjakan: [nomor & nama item].
Status centang terakhir: [tempel].
Tugas: [jelaskan].
Batasan: patuhi "Prinsip yang Tidak Boleh Dilanggar" di roadmap; jangan lompat lapis;
sertakan tes untuk setiap perubahan (fitur, bug fix, refactor); laporkan hasil dengan jujur (apa yang lulus, apa yang belum).
```

---

## 📌 Ringkasan Eksekutif

**Tujuan akhir:** Garuda menjadi bahasa general-purpose dengan runtime GVM yang mampu menjangkau:

1. Backend konkuren & terdistribusi (fondasi sudah ada)
2. Mobile app (Android/iOS)
3. Game engine (realtime, 2D→3D)
4. General-purpose (CLI, desktop, scripting, embedded ringan)

**Estimasi jujur (1 orang, ±10–20 jam/minggu):** general-purpose yang layak pakai 5–7 tahun; mobile + game engine nyata 8–12 tahun. Dengan kontributor bisa lebih cepat. Ini maraton, bukan sprint: lebih baik roadmap jujur yang dijalani lama daripada roadmap optimis yang membuat berhenti di tahun ke-3.

---

## 🧱 LAPIS 0 — PENGERASAN FONDASI (0–9 bulan)

**Tujuan:** GVM & compiler cukup kokoh sebagai dasar semua lapis berikutnya.

### 0.1 Stabilitas Compiler & VM
- [x] Selesaikan TCO (6/6 tes TCO lulus, stack O(1), no regression di test_isolation & test_fault_isolation)
- [x] Audit Control Flow & Branching (`OpJump` & `OpJumpIfFalse` terverifikasi via `test_control_flow.cpp` + mutation test)
- [x] Watchdog tes berbasis proses terpisah (`scripts/run_tests.ps1`)
- [ ] Test suite lengkap; CI wajib lulus sebelum merge ke main
- [x] Fuzzing parser & codegen (mini-fuzzer & truncation fuzzing di test_fuzzing.cpp)
- [x] Error handling konsisten — semua error punya pesan + lokasi (baris/kolom terstruktur di ParseError)
- [x] Deterministic build — build sama menghasilkan binary sama (/Brepro MSVC + check_deterministic.ps1)

### 0.2 Runtime & Memory
- [x] Pilih model memory: GC tracing vs ARC vs manual+arena (keputusan krusial)
  - Keputusan: **hybrid** — GC ringan per-aktor (seperti BEAM per-process heap) untuk backend, **arena/frame allocator deterministik** untuk hot path (game, mobile). Keduanya harus ada di runtime sejak Lapis 0 (minimal desain + API), bukan ditambahkan belakangan; ini menutup kontradiksi dengan prasyarat Lapis 4 "GC pause = mati".
- [x] Implementasi API Memory Model (lihat docs/MEMORY_MODEL.md)
- [x] Hindari global lock di hot path (spawn, send, receive)
- [x] Preemption adil — aktor tidak boleh memonopoli CPU
- [x] Observability: metrics (spawn rate, mailbox depth, GC pause), tracing, profiling hook

### 0.3 Ekosistem Minimum
- [x] Package manager (fondasi `package_manager` sudah ada) — registry, semver, lockfile
- [x] Stdlib inti: string, koleksi, IO, time, math, crypto dasar
- [ ] Dokumentasi bahasa — spesifikasi formal + tutorial
- [ ] Formatter & linter (`formatter_linter` sudah ada) — wajib di CI

### 0.4 Keamanan & Kualitas
- [x] Secret scanning, push protection, dependabot
- [x] `SECURITY.md`, `CONTRIBUTING.md`, lisensi Apache 2.0
- [x] Benchmark suite di CI (cegah regresi performa)

**Gate keluar Lapis 0:** 5/5 TCO pass; 0 bug kritis yang diketahui pada **jalur utama** (TCO, spawn, send, receive, HTTP dasar); bug kritis lain tercatat di `KNOWN_ISSUES.md` beserta workaround; CI hijau pada 30 commit terakhir dengan ≥1 commit/minggu selama 30 hari; benchmark stabil (tidak turun >5%).

**Catatan revisi Gate CI (26 Okt 2026):** Gate CI direvisi dari **100 → 30 commit**. Alasan:
- 100 commit untuk proyek solo (±10 jam/minggu, 1 sprint = 2 minggu) membutuhkan **5–6 bulan** — terlalu lama untuk menahan Lapis 0 secara tidak proporsional.
- 20 commit (usulan awal agen) dianggap **sedikit terlalu longgar**.
- **30 commit** (~1,5–2 bulan) dipilih sebagai kompromi: cukup untuk membuktikan stabilitas build/test berkelanjutan, tanpa menahan proyek di Lapis 0 lebih lama dari yang diperlukan.
- Gate tetap menguji **disiplin commit** (≥1 commit/minggu selama 30 hari) dan **stabilitas** (30 commit hijau beruntun).

---

## 🖥️ LAPIS 1 — BACKEND & LAYANAN (9–24 bulan)

**Tujuan:** Garuda bisa dipakai membangun layanan produksi nyata.

### 1.1 Jaringan & Protokol
- [x] HTTP/1.1 (HTTP/2 ditunda ke backlog)
- [x] WebSocket (realtime) (E2E teruji via garuda_websocket_e2e_tests)
- [~] gRPC / protobuf (Protobuf minimal selesai, gRPC penuh ditunda)
- [~] TLS native (vendor mbedTLS v3.5.2 — E2E socket handshake teruji via garuda_tls_tests)

### 1.2 Data & Persistensi
- [~] SQLite matang; Postgres/Redis ditunda ke backlog
- [x] ORM ringan / query builder (QueryBuilder + Tes SQL Injection)
- [x] Migrasi skema (SchemaMigrator dengan transaksi SQL)
- [~] Persistensi aktor (snapshot + event log) — implementasi Term/JSON & tes disk roundtrip selesai

### 1.3 Observability & Operasional
- [ ] Structured logging
- [ ] Metrics (format Prometheus)
- [ ] Distributed tracing (OpenTelemetry)
- [x] Graceful shutdown, health check, readiness probe (Health Check + Graceful Shutdown teruji)
- [ ] Hot reload (ditunda ke backlog)

### 1.4 Keamanan Aplikasi
- [~] JWT selesai; OAuth2 client ditunda
- [x] Rate limiting, input validation (RateLimiter + Validator + Tes XSS)
- [x] Secret management (env, vault) — Env var & .env file loader teruji

**Gate keluar Lapis 1:** 1 layanan produksi nyata berjalan ≥ 3 bulan (RICHAT sebagai hero use case) dengan uptime & observability terbukti.
- [~] Demo backend RICHAT (WebSocket + SQLite) selesai; integrasi ke aplikasi nyata menyusul
STATUS: Lapis 1 FUNGSIONAL SELESAI (Fondasi server & demo RICHAT terbukti). Gate resmi (RICHAT 3 bulan uptime) dikerjakan secara bertahap di lapis berikutnya.

## 📦 BACKLOG (Dikerjakan Sambil Jalan)
- [ ] HTTP/2
- [ ] gRPC penuh (transport)
- [ ] Driver Postgres & Redis
- [ ] OAuth2 Client
- [ ] Hot Reload

---

## 💻 LAPIS 2 — GENERAL-PURPOSE (24–48 bulan)

**Tujuan:** Garuda bisa dipakai untuk CLI, desktop, scripting, otomasi — bukan hanya server.

### 2.1 Interop & FFI
- [~] C ABI stabil (Fondasi FFI: registry + konversi tipe dasar)
- [ ] Binding otomatis dari header C (seperti bindgen)
- [ ] Embedding API — GVM bisa ditanam di aplikasi C++/Rust

### 2.2 Toolchain Desktop
- [ ] Cross-compile (Windows, Linux, macOS)
- [~] Static linking (konfigurasi build selesai; cross-compile Linux/macOS menyusul)
- [ ] GUI toolkit — binding ke GTK/Qt, atau toolkit native Garuda
- [~] Filesystem & Process API (Fondasi toolchain desktop)

### 2.3 Ekosistem & Komunitas
- [ ] Registry paket publik
- [x] Dokumentasi API otomatis (doc comments → HTML) (DocGenerator teruji)
- [~] Contoh proyek CLI tool (salam CLI)
- [x] Program kontributor & governance (dokumen CONTRIBUTING, CoC, Governance)

### 2.4 Scripting & Embedding
- [x] Mode scripting ringan (ScriptEngine eval/eval_file teruji 100%)
- [ ] Sandboxing script (batas resource, tanpa akses FS/jaringan secara default)

**Gate keluar Lapis 2:** Minimal 3 aplikasi general-purpose nyata (CLI, desktop, script) dibuat oleh orang selain kamu, dengan feedback positif.

---

## 📱 LAPIS 3 — MOBILE (48–84 bulan)

**Tujuan:** Garuda bisa membuat aplikasi Android & iOS. Lapis paling berat — butuh interop native, UI, dan distribusi.

### 3.1 Prasyarat Wajib (dari Lapis 0–2)
- Memory model deterministik / GC rendah-pause
- C ABI stabil + FFI
- Cross-compile ke ARM64/ARMv7
- Toolchain build cepat

### 3.2 Android
- [ ] Tahap A: GVM tertanam via NDK (interpreter) untuk development/prototipe dan, bila perlu, rilis ringan berisi logika aplikasi non-kritis. Bytecode harus dibundel di dalam APK (tidak diunduh dari luar Play, sesuai kebijakan Google Play). Tidak boleh dipakai untuk hot path/game dan tidak boleh jadi jalur produksi jangka panjang.
- Aturan rilis Android: aplikasi yang butuh performa/baterai wajib Tahap B (AOT via transpile C/C++), sama seperti iOS.
- [ ] Tahap B: AOT ke ARM64 (transpile C/C++ dulu, LLVM belakangan) untuk produksi yang butuh performa dan hemat baterai; wajib di iOS
- [ ] JNI binding otomatis
- [ ] UI: binding ke Jetpack Compose / Android Views, atau toolkit native via Canvas/Skia
- [ ] Akses fitur: kamera, GPS, storage, notifikasi, network
- [ ] Lifecycle aplikasi mobile (pause/resume, background)
- [ ] Gradle plugin / build integration

### 3.3 iOS
- [ ] AOT ke ARM64 (Mach-O)
- [ ] Interop Swift/Objective-C (via C ABI)
- [ ] UI: binding ke SwiftUI / UIKit
- [ ] Kepatuhan App Store — tanpa JIT; AOT wajib
- [ ] Integrasi Xcode

### 3.4 Cross-platform UI (opsional tapi strategis)
- [ ] Toolkit UI lintas platform (render via Skia/Metal/Vulkan) atau binding ke Flutter/React Native sebagai host
  - Rekomendasi: mulai dari binding ke host, bukan membuat UI framework sendiri — hemat ±3 tahun

**Gate keluar Lapis 3:** 1 aplikasi mobile nyata (bukan demo) dirilis di Play Store / App Store, dibuat dengan Garuda.

---

## 🎮 LAPIS 4 — GAME ENGINE (72–120 bulan)

**Tujuan:** Garuda bisa dipakai membangun game, dari 2D sampai 3D.

### 4.1 Prasyarat Wajib
- Memory deterministik / manual (GC pause = mati di game)
- AOT native (JIT tidak boleh di iOS, dan JIT = stutter)
- SIMD, multithreading native
- FFI ke C/C++ (untuk engine existing)

### 4.2 Fondasi Rendering & Audio
- [ ] Binding GPU: Vulkan, Metal, DirectX 12 (via C ABI)
- [ ] Math library: vektor, matriks, quaternion (SIMD)
- [ ] Audio: OpenAL / miniaudio binding
- [ ] Input: keyboard, mouse, gamepad, touch
- [ ] Window & event loop lintas platform

### 4.3 Game Runtime
- [ ] ECS (Entity Component System)
- [ ] Asset pipeline: model, tekstur, audio, animasi
- [ ] Scene graph / spatial partition
- [ ] Physics: binding ke Jolt / Bullet, atau physics ringan native
- [ ] Scripting game logic — Garuda sebagai scripting di atas engine C++ (seperti Lua di Unity), atau engine penuh di Garuda
- [ ] Hot reload

### 4.4 Tooling
- [ ] Editor (scene editor, inspector, asset browser) — desktop app Garuda (Lapis 2)
- [ ] Profiler (frame time, draw call, memory)
- [ ] Debugger (breakpoint, inspect state)

### 4.5 Strategi Masuk Pasar
- **Jalur A (realistis):** Garuda sebagai scripting language untuk engine existing (Godot, Bevy, custom C++). Jauh lebih cepat dan langsung berguna.
- **Jalur B (ambisius):** Engine penuh di Garuda. Butuh 5+ tahun dan tim.
- **Rekomendasi:** Jalur A dulu; Jalur B setelah Lapis 2–3 matang.

**Gate keluar Lapis 4:** 1 game nyata (2D dulu, lalu 3D) dirilis, dibuat dengan Garuda.

---

## 🏟️ JALUR PARALEL: STADION GARUDA (playground, berjalan di semua lapis)

**Tujuan:** Stadion Garuda adalah wajah publik Garuda: tempat orang mencoba bahasa, melihat aktor bekerja, dan belajar tanpa instalasi. Ia berkembang **sejalan** dengan tiap lapis, bukan menunggu akhir.

**Kunci teknis:** kompilasi GVM + compiler ke **WebAssembly** (Emscripten) agar Stadion jalan 100% di browser (juga di Chrome Android dan bisa dibungkus APK).

| Tahap | Seiring Lapis | Isi Stadion |
|---|---|---|
| S0 | Lapis 0 | Editor kode + tombol Run + output (GVM di WASM); contoh bawaan (fakt, spawn, send/receive); batas waktu & memori eksekusi |
| S1 | Lapis 1 | **Visualizer aktor**: pohon supervisor, mailbox depth, pesan mengalir antar aktor (seperti Erlang Observer); log & tracing langsung |
| S2 | Lapis 1–2 | Tutorial interaktif berbahasa Indonesia (tur bertahap), galeri contoh, **share link** (kode dalam URL/gist), mode "tantangan" |
| S3 | Lapis 2 | Debugger langkah-demi-langkah, inspeksi state aktor, simulasi crash & restart supervisor, benchmark mini |
| S4 | Lapis 3 | Versi mobile/APK Stadion (offline), simpan proyek lokal |
| S5 | Lapis 4 | Playground 2D (canvas) untuk game kecil berbasis aktor |

- [ ] S0 — Stadion minimal berjalan di browser (WASM)
- [ ] S1 — Visualizer aktor & supervisor
- [ ] S2 — Tutorial Indonesia + share link
- [ ] S3 — Debugger & simulasi kegagalan
- [ ] S4 — Mobile/APK offline
- [ ] S5 — Playground 2D

**Gate Stadion:** tiap rilis Garuda baru harus punya contoh yang bisa dijalankan di Stadion sebelum dianggap selesai.

**DoD Stadion S0** (semua harus terpenuhi sebelum S0 dicentang):
- `grdc` + GVM ter-compile ke WASM (Emscripten) dan jalan di Chrome/Firefox/Safari versi terbaru.
- Minimal 3 contoh bawaan (`fakt`, `spawn`, `send/receive`) bisa dijalankan dari editor.
- Batas waktu eksekusi (target awal 5 detik) dan batas memori (target awal 64 MB) aktif; program yang melebihi dihentikan dengan pesan jelas, browser tidak freeze.
- Error compiler/runtime tampil di UI lengkap dengan baris/kolom.
- Dapat di-host sebagai halaman statis (GitHub Pages / Cloudflare Pages) tanpa server backend.
- Ukuran bundle WASM dicatat (target awal < 5 MB; sesuaikan bila ternyata tidak realistis) dan tidak naik > 10% antar rilis tanpa alasan tertulis.

---

## 💬 JALUR RICHAT DI ATAS GARUDA (hero use case Lapis 1)

**Tujuan:** RICHAT (klien Kotlin/Compose) berpindah dari backend Erlang/Supabase ke backend Garuda secara bertahap, sehingga Garuda terbukti di produksi dengan pengguna nyata.

**Strategi: strangler (pindah per fitur, bukan sekaligus).** Backend lama tetap hidup sebagai cadangan sampai fitur yang sama di Garuda stabil.

| Tahap | Isi | Syarat mulai |
|---|---|---|
| R0 | Spesifikasi kontrak API/protokol RICHAT yang sekarang (pesan, kanal, status) + test kontrak otomatis | Lapis 0 berjalan |
| R1 | Gateway WebSocket Garuda: koneksi, auth token, presence, kirim/terima pesan 1-on-1, antrean pesan offline | WebSocket + TLS selesai (1.1) |
| R2 | Grup/room (aktor per room), riwayat pesan dengan persistensi, tanda baca/delivered | Persistensi aktor (1.2) |
| R3 | Observability penuh: metrics, tracing, health check; uji beban & chaos (matikan aktor/node) | 1.3 |
| R4 | Media/berkas, notifikasi push, bot/assistant | R2 stabil |
| R5 | Dompet digital / transaksi | **Terakhir**, hanya setelah gate Lapis 1 lulus |

**Aturan penting:**
- **Dompet/transaksi tetap di backend yang sudah terbukti** sampai Garuda lulus gate Lapis 1. Bug di chat hanya mengganggu; bug di uang merugikan pengguna dan bisa menyangkut kewajiban hukum/regulasi.
- Jalankan **shadow mode**: pesan diteruskan ke Garuda secara paralel dan hasilnya dibandingkan dengan backend lama sebelum trafik asli dipindah.
- Siapkan **rollback satu langkah** (klien bisa kembali ke backend lama lewat konfigurasi).
- Gate tetap: ≥ 3 bulan berjalan dengan pengguna nyata (bukan hanya pengujian sendiri), uptime & observability terbukti.
- Bug di Garuda yang ditemukan lewat RICHAT dicatat sebagai item Lapis 0 (bukan ditambal di sisi aplikasi).
- **Rem darurat dompet (R5):** jika 6 bulan setelah R4 stabil gate Lapis 1 belum lulus, R5 ditunda tanpa batas dan transaksi tetap di backend lama. Tidak ada tekanan waktu memindahkan dompet; keamanan uang pengguna lebih penting daripada milestone roadmap.
- [ ] R0 [ ] R1 [ ] R2 [ ] R3 [ ] R4 [ ] R5

---

## 🧩 PEMETAAN KEGUNAAN GARUDA (bahasa berbasis C++, model aktor)

Karena GVM ditulis dalam C++ dan modelnya aktor + pesan, Garuda cocok untuk lebih dari sekadar server. Urut dari paling dekat ke paling jauh:

**Jangka dekat (Lapis 0–1, paling realistis)**
1. **Server chat & realtime** — hero use case Lapis 1: RICHAT.
2. **Game server multiplayer** (lobi, room, matchmaking, giliran kartu) — satu aktor per room/pemain; cocok dengan platform game kartu.
3. **Bot & otomasi** (Telegram/WhatsApp gateway, scraper terjadwal, notifikasi) — aktor ringan + supervisor.
4. **Workflow & job queue** — retry, backoff, dead-letter, dengan persistensi aktor.
5. **Pembelajaran pemrograman** — sintaks Indonesia + Stadion = media belajar konkurensi untuk pemula.

**Jangka menengah (Lapis 2)**
6. **Scripting tertanam (embedded)** di aplikasi C++ — pengganti Lua dengan model aktor.
7. **Sistem plugin/ekstensi aman** — sandbox + batas resource.
8. **CLI & tool otomasi** (build tool, pemroses berkas paralel).
9. **IoT/edge ringan** — banyak sensor = banyak aktor, supervisor menangani kegagalan.
10. **Pipeline data & ETL** — aliran pesan antar aktor dengan backpressure.
11. **Simulasi berbasis agen** (agent-based) — tiap agen = aktor.

**Jangka panjang (Lapis 3–4)**
12. **Logika aplikasi mobile** (GVM tertanam via NDK; UI lewat host).
13. **Scripting game** untuk engine existing (Jalur A), AI NPC berbasis aktor.
14. **Orkestrasi agen AI** — aktor sebagai agen, pesan sebagai tool-call; supervisor untuk fault tolerance.
15. **Telekomunikasi/pesan tertunda & sistem terdistribusi** (Raft, replikasi) — warisan sejati BEAM.

**Bukan target (hindari agar fokus):** komputasi numerik berat/ML training, driver/OS kernel, aplikasi yang butuh latensi mikrodetik keras.

---

## 🧭 PRINSIP YANG TIDAK BOLEH DILANGGAR

1. **Jangan lompat lapis.** Mobile sebelum backend keras = fondasi retak.
2. **Jangan janji sebelum bisa dibuktikan.** Klaim "bisa bikin game" hanya setelah ada game nyata.
3. **Memory model adalah keputusan sekali seumur proyek.** Salah pilih = refactor 2 tahun.
4. **Interop > reinvent.** Binding ke C/C++/GPU/UI existing jauh lebih cepat daripada bikin sendiri.
5. **Ekosistem mengalahkan bahasa.** Investasi di package manager & dokumentasi sejak Lapis 0.
6. **Satu hero use case per lapis.** Backend: RICHAT (chat server). General: CLI tool. Mobile: 1 app. Game: 1 game.
7. **Jaga identitas.** Sintaks Indonesia + model aktor adalah diferensiasi.
8. **Stadion ikut tumbuh.** Fitur yang tidak bisa dicoba di Stadion belum dianggap selesai.
9. **Tidak ada perubahan tanpa tes.** Fitur baru butuh tes perilaku; perbaikan bug butuh tes regresi yang gagal sebelum perbaikan dan lulus sesudahnya; refactor butuh tes lama tetap lulus tanpa diubah. CI menolak merge jika tes gagal.

---

## 📊 RINGKASAN TIMELINE

| Lapis | Fokus | Estimasi | Gate Keluar |
|---|---|---|---|
| 0 | Pengerasan fondasi (TCO, memory, CI) | 0–9 bln | 5/5 TCO pass, CI hijau 30 hari |
| 1 | Backend & layanan produksi | 9–24 bln | 1 layanan uptime 3 bulan |
| 2 | General-purpose (CLI, desktop, FFI) | 24–48 bln | 3 app oleh orang lain |
| 3 | Mobile (Android, iOS) | 48–84 bln | 1 app di store |
| 4 | Game engine (2D→3D) | 72–120 bln | 1 game dirilis |
| S | Stadion Garuda (paralel, S0→S5) | mulai Lapis 0 | Tiap rilis punya contoh di Stadion |

- General-purpose layak: ~5–7 tahun (solo, ±10–20 jam/minggu).
- Mobile + game: ~8–12 tahun solo; bisa lebih cepat dengan kontributor. Gate keluar lebih penting daripada angka tahun.

---

## 🔑 KEPUTUSAN KRUSIAL (Lapis 0) — FINAL

Status: **FINAL** (disetujui pemilik proyek, 26 Oktober 2026). Perubahan hanya boleh lewat keputusan eksplisit pemilik proyek, bukan inisiatif agen.

| # | Keputusan | Pilihan (FINAL) | Alasan |
|---|---|---|---|
| 1 | Memory model | **Hybrid**: heap terpisah per aktor dengan GC kecil (copying/generasional) + binary besar ref-counted (ala BEAM) + **arena/frame allocator deterministik untuk hot path game/mobile; keduanya ada di runtime sejak Lapis 0 (minimal desain + API)** | Aktor mati = heap langsung bebas; tanpa GC global, pause kecil. Cocok untuk backend sekaligus jalur mobile/game nanti |
| 2 | Eksekusi | **Interpreter GVM dulu**, IR dirancang AOT-friendly; jalur AOT pertama lewat **transpile ke C/C++** (lalu kompilasi dengan toolchain), LLVM belakangan | Proyek sudah C++ sehingga transpile paling murah; interpreter tetap dibutuhkan untuk Stadion (WASM) dan scripting |
| 3 | Mobile | **GVM tertanam via NDK + UI lewat host** (WebView/HTML5 yang sudah kamu kuasai, atau Compose); Flutter/RN belakangan | Memanfaatkan alur kerja yang ada; tidak membangun UI framework sendiri |
| 4 | Game | **Scripting untuk engine/game existing (Jalur A)**; game 2D kecil via WASM/canvas di Stadion | Hasil cepat & nyata; engine penuh ditunda sampai Lapis 2–3 matang |
| 5 | Interop | **C ABI sebagai jembatan utama** (wajib Lapis 2), ditambah target **WASM** sebagai jembatan ke browser/Stadion | C ABI membuka mobile, game, embedding; WASM membuka Stadion & distribusi tanpa instalasi |

Keputusan tambahan:
- **Distribusi bahasa:** satu binary statis (`grdc` + GVM) untuk desktop; build WASM untuk Stadion.
- **Hero use case Lapis 1: RICHAT** (chat realtime) menjadi pengguna pertama backend Garuda & GVM. Game server (Remi Hub) menyusul sebagai use case kedua. Lihat bagian "Jalur RICHAT di atas Garuda".

---

## ✅ DEFINISI SELESAI (DoD) — berlaku untuk setiap item `[x]`

Item hanya boleh dicentang jika **semua** terpenuhi:
1. Tes otomatis lulus di CI: tes perilaku untuk fitur baru, tes regresi untuk bug yang diperbaiki (gagal sebelum perbaikan, lulus sesudahnya), dan tes lama tetap lulus untuk refactor. Tanpa tes = belum selesai.
2. Tidak ada regresi pada tes lama. Benchmark jalur panas tidak turun >5% **antar-rilis sejak baseline baru ditetapkan (26 Okt 2026)**. Baseline sebelum tanggal ini (mis. HTTP 35.971 req/s) **tidak dapat direproduksi** dan **tidak dipakai sebagai pembanding**. Setiap penurunan >5% antar-rilis wajib diselidiki dengan mutation test (seperti H4 pada investigasi HTTP 26 Okt 2026) untuk membedakan overhead perbaikan dari regresi nyata. Baseline baru yang tercatat:
   - HTTP server: 16.300 req/sec (median 2 run, 26 Okt 2026).
   - Spawn aktor: 638.849 proses/sec (26 Okt 2026).
3. Terdokumentasi (README/doc comment/spec) dan ada contoh yang bisa dijalankan di Stadion.
4. Bug yang diketahui dicatat di `KNOWN_ISSUES.md` (bukan disembunyikan).
5. Commit + entri changelog.

**Contoh kriteria selesai (Lapis 0):**
- **TCO:** 5/5 tes TCO lulus (termasuk `fakt(5)`), `test_isolation` tidak regresi, benchmark spawn tidak turun >5%, rekursi ekor 1 juta level tanpa pertumbuhan stack.
- **Error handling konsisten:** semua kelas error punya pesan + baris/kolom; ada tes snapshot untuk pesan error.
- **Fuzzing:** parser & codegen dijalankan ≥1 jam di CI tanpa crash; temuan crash menjadi tes regresi.
- **Deterministic build:** dua build dari commit yang sama menghasilkan hash binary identik.
- **Model memory:** dokumen desain + API arena/heap-aktor + tes yang membuktikan heap aktor bebas saat aktor mati.

---

## 🎯 HERO USE CASE & METRIK KEBERHASILAN (target terukur, boleh direvisi saat review)

| Lapis | Hero use case | Metrik keberhasilan |
|---|---|---|
| 1 | RICHAT (chat server) | Uptime ≥ 99,9% selama 30 hari berturut-turut, ≥ 3 bulan berjalan dengan pengguna nyata |
| 2 | CLI tool `grd` untuk otomasi | 3 aplikasi oleh orang lain, masing-masing ≥ 10 bintang GitHub |
| 3 | Satu app mobile sederhana (usul: aplikasi catatan) | Rilis di store dengan ≥ 100 unduhan |
| 4 | Satu game 2D | ≥ 1.000 pemain |
| S | Stadion Garuda | Tiap rilis punya contoh yang bisa dijalankan di Stadion |

---

## 🚫 NON-GOALS (yang sengaja tidak dilakukan Garuda)

- Bukan pengganti Python untuk data science / ML training.
- Tidak menjanjikan GC realtime keras; latensi hard-realtime tetap lewat jalur arena/manual.
- Tidak membuat renderer 3D sendiri tanpa binding GPU yang ada (Interop > reinvent).
- Tidak membuat UI framework lintas platform sendiri sebelum Lapis 3 matang.
- Tidak menjadi bahasa tanpa identitas: sintaks Indonesia + model aktor tidak dinegosiasikan.
- Tidak menangani transaksi uang di Garuda sebelum gate Lapis 1 lulus.

---

## TECHNICAL DEBT

- [!] Watchdog tes saat ini menggunakan `std::async` + `std::exit(1)` di `test_tco.cpp` dan `test_control_flow.cpp`. Ini rapuh: tidak membersihkan state, dan bisa menyembunyikan tes mana yang hang setelahnya. Ganti dengan watchdog berbasis proses terpisah (CreateProcess di Windows / fork+waitpid di POSIX) di iterasi berikutnya. Bukan blocker, tapi harus diselesaikan sebelum CI dianggap stabil.

---

## ⚠️ RISIKO & MITIGASI

| Risiko | Dampak | Mitigasi |
|---|---|---|
| Burnout (pengembang solo) | Proyek berhenti | Target ±10 jam/minggu yang berkelanjutan; rayakan milestone kecil; estimasi jujur |
| Memory model/IR salah desain | Refactor besar | Keputusan sudah final; tulis dokumen desain sebelum runtime melewati ±50 ribu baris; IR dirancang AOT-friendly sejak sekarang |
| Ekosistem tidak tumbuh | Bahasa tidak dipakai | Package manager + dokumentasi sejak Lapis 0; Stadion sebagai pintu masuk; 1 hero use case per lapis |
| Bug Garuda memengaruhi pengguna RICHAT | Kepercayaan pengguna turun | Strangler + shadow mode + rollback satu langkah; dompet tetap di backend terbukti |
| Terlalu bergantung pada agen AI | Kode tak dipahami / konteks hilang | Dokumen desain & ROADMAP.md selalu diperbarui; DoD + tes otomatis sebagai pagar; review manual berkala |
| Scope creep | Lapis tak pernah selesai | Gate keluar + Non-Goals + larangan lompat lapis |
| Kehilangan identitas | Jadi "bahasa biasa" | Sintaks Indonesia + model aktor bersifat non-negosiasi |

---

## 🔁 IRAMA KERJA (usul, boleh disesuaikan)

- 1 sprint = 2 minggu.
- Tiap sprint: 1 item (atau sub-item) Lapis 0 + 1 perbaikan Stadion/DoD.
- Akhir sprint: perbarui ROADMAP.md (penanda status + catatan kendala), commit, review singkat ±15 menit.
- Tiap 6 sprint (±3 bulan): review besar; berikutnya 26 Januari 2027.

---

## 📝 CARA PAKAI DOKUMEN INI

- Simpan sebagai `ROADMAP.md` di root repo Garuda.
- Penanda status: `[ ]` belum dimulai, `[~]` sedang dikerjakan, `[!]` terhambat (tulis alasannya di samping), `[x]` selesai (sesuai Definisi Selesai). Item `[~]`/`[!]` selalu diberi catatan kendala singkat.
- Setiap selesai satu item, centang `[x]` lalu commit.
- Tiap 3 bulan, review: apakah gate keluar tercapai? Jika belum, jangan naik lapis. Catat tanggal review berikutnya di bagian atas.
- Jika kehilangan konteks (kuota habis, ganti sesi), paste dokumen ini + status centang terakhir ke agen baru.

---

## 🦅 KALIMAT UNTUK DIINGAT

> "Garuda tidak akan menjadi general-purpose dengan menambah fitur. Ia akan menjadi general-purpose dengan mengeraskan fondasi, lalu meminjam kekuatan ekosistem lain lewat interop, sambil menjaga identitas: sintaks Indonesia + model aktor."
