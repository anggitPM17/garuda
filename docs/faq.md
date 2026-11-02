# FAQ — Pertanyaan Umum tentang Bahasa Garuda

Jawaban resmi atas pertanyaan yang paling sering diajukan mengenai Bahasa Garuda v67.0.0.

---

## 1. Umum & Filosofi

### Apa itu Bahasa Garuda?
Bahasa Garuda adalah bahasa pemrograman fungsional modern berkinerja tinggi berbasis *Actor Model Concurrency* bergaya Erlang/Elixir yang dieksekusi di atas Garuda Virtual Machine (GVM). 100% karya anak bangsa Indonesia 🇮🇩.

### Apakah Bahasa Garuda v67.0.0 Siap Digunakan (*Production-Ready*)?
Ya. Versi v67.0.0 telah mencapai status stabil dengan **100% PASS** pada 14 suite pengujian unit C++ (0 Fail, 0 Skip). Seluruh modul pustaka standar (`std/*.grd`), Web Framework (`std/web.grd`), dan tooling IDE (LSP & VS Code Extension) berfungsi penuh secara stabil.

### Berapa Lisensi Bahasa Garuda?
Bahasa Garuda dilesensikan di bawah **Apache License 2.0** atas nama hak cipta **Anggit Prasetyo Mituhu**. Bebas digunakan, dimodifikasi, dan didistribusikan untuk keperluan komersial maupun non-komersial.

---

## 2. Performa & Benchmark

### Seberapa Cepat Eksekusi Bahasa Garuda?
Berdasarkan pengukuran benchmark resmi pada Windows 10 x64 (MSVC 2022 Release build):
- **Startup Time**: **0.032 ms** (32 mikrodetik!)
- **Actor Spawn & Send**: **0.071 ms** (71 mikrodetik!)
- **Map Operations**: **1.086 ms**
- **List.peta Transformation**: **4.236 ms**
- **JSON Serialization**: **7.661 ms**

Laporan lengkap dapat dibaca pada **[benchmarks/RESULTS.md](../benchmarks/RESULTS.md)**.

---

## 3. Fitur & Kapabilitas

### Apakah Bahasa Garuda Bisa Digunakan untuk Web Backend / REST API?
Sangat Bisa! Bahasa Garuda menyediakan Web Framework murni `std/web.grd` dengan dukungan routing dinamis GET/POST dan eksekusi handler Garuda secara *thread-safe* di atas HTTP Server.

### Apakah Bahasa Garuda Mendukung Concurrency?
Ya! Bahasa Garuda mengadopsi **Actor Model Concurrency** yang sama seperti Erlang/OTP. Anda dapat membangkitkan proses baru menggunakan `cakra`, mengirim pesan dengan `kirim`, dan menerima pesan pada mailbox menggunakan `tampa`.

---

## 4. IDE Tooling & Ekosistem

### Bagaimana Cara Mengaktifkan Autocomplete & Hover di VS Code?
Kompilasi LSP Server (`lsp/`) dan VS Code Client Extension (`vscode-extension/`), lalu salin folder ke direktori `.vscode/extensions/` Anda. Lihat panduan lengkap pada **[docs/lsp-vscode.md](lsp-vscode.md)**.

### Di mana Saya Bisa Melihat Contoh Program Nyata?
Repositori menyediakan 6 contoh aplikasi nyata pada direktori `examples/`:
- `examples/todo_api/` — REST API CRUD
- `examples/chat_server/` — Chat Server dengan Actor Concurrency
- `examples/data_analysis/` — Data Pipeline
- `examples/json_processor/` — Pemrosesan JSON
- `examples/calculator/` — Kalkulator Fungsional
- `examples/hello_world.grd` — Hello World

---

## 5. Kontribusi & Komunitas

### Bagaimana Cara Berkontribusi?
Silakan baca panduan lengkap kontribusi pada **[CONTRIBUTING.md](../CONTRIBUTING.md)**. Seluruh *Pull Request* disambut hangat di repositori GitHub **[anggitPM17/garuda](https://github.com/anggitPM17/garuda)**.
