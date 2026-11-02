# Getting Started — Bahasa Garuda dalam 10 Menit

Selamat datang di Bahasa Garuda v67.0.0! Panduan ini dirancang untuk membantu Anda mengompilasi runtime, mengeksekusi program Garuda pertama Anda, dan memahami alur pengembangan dalam waktu kurang dari 10 menit.

---

## Prasyarat Lingkungan

Sebelum memulai, pastikan perangkat Anda memiliki:

- **Sistem Operasi**: Windows 10/11, Linux (Ubuntu 20.04+), atau macOS.
- **CMake**: Versi 3.20 atau lebih baru.
- **Compiler C++20**:
  - Windows: Visual Studio 2022 (MSVC) dengan *Desktop Development with C++*.
  - Linux: GCC 10+ atau Clang 12+.
  - macOS: Xcode Command Line Tools.
- **Node.js**: Versi 18+ (*opsional, hanya jika ingin mengaktifkan LSP & Ekstensi VS Code*).

---

## Langkah 1: Kompilasi Runtime Garuda CLI (`grd`) (5 Menit)

1. Clone repositori resmi Bahasa Garuda:
   ```powershell
   git clone https://github.com/anggitPM17/garuda.git
   cd garuda/garuda_v67_0_production/garuda_v67.0.0
   ```

2. Konfigurasi dan kompilasi target `grd` menggunakan CMake:
   ```powershell
   # Windows MSVC
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release --target grd
   ```

   *Untuk Linux / macOS*:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release --target grd -j$(nproc)
   ```

   Executable `grd` akan terbentuk di lokasi `build/Release/grd.exe` (Windows) atau `build/grd` (Linux/macOS).

---

## Langkah 2: Program Pertama — Hello World (2 Menit)

Buat berkas skrip baru bernama `halo.grd`:

```garuda
modul Halo

fungsi utama() ->
    "Halo dari Bahasa Garuda v67.0.0!"
rampung
```

Jalankan skrip menggunakan CLI Tooling `grd`:

```powershell
.\build\Release\grd.exe run halo.grd
```

**Output Terminal**:
```text
=> "Halo dari Bahasa Garuda v67.0.0!"
```

🎉 **Selamat!** Anda telah berhasil menjalankan skrip Bahasa Garuda pertama Anda.

---

## Langkah 3: Verifikasi Suite Pengujian (1 Menit)

Jalankan suite pengujian otomatis untuk memastikan seluruh komponen runtime berfungsi 100%:

```powershell
.\build\Release\grd.exe test
```

**Output Terminal**:
```text
======================================================
RINGKASAN HASIL TEST: 14 PASS | 0 FAIL | 0 SKIP
======================================================
```

---

## Langkah 4: Membuat Proyek Baru (1 Menit)

Gunakan perintah `grd new` untuk membuat struktur proyek baru secara otomatis:

```powershell
.\build\Release\grd.exe new proyek_saya
cd proyek_saya
..\build\Release\grd.exe run utama.grd
```

**Output Terminal**:
```text
=> "Halo dari proyek proyek_saya!"
```

---

## Langkah 5: Langkah Selanjutnya

- 📘 **[Tutorial Pertama (docs/tutorial-pertama.md)](tutorial-pertama.md)** — Bangun aplikasi Todo List CLI dalam 30 menit.
- 📖 **[Sintaks Bahasa (docs/sintaks.md)](sintaks.md)** — Pelajari `yen/lain`, `nalika`, `|>` pipe operator, dan Actor `cakra`.
- 📦 **[Pustaka Standar (docs/pustaka-standar.md)](pustaka-standar.md)** — Referensi modul `List`, `String`, `Map`, `Tuple`, `Json`, `Waktu`, `Matematika`.
- 🌐 **[Web Framework (docs/web-framework.md)](web-framework.md)** — Pelajari pembuatan RESTful API backend.
- 🎨 **[LSP & VS Code (docs/lsp-vscode.md)](lsp-vscode.md)** — Aktifkan autocomplete 68 simbol dan hover documentation.

---

## Pertanyaan & Masalah Umum (*Troubleshooting*)

- **"Error: File 'halo.grd' tidak ditemukan!"**: Pastikan Anda menjalankan `grd run` di direktori yang sama dengan berkas `.grd`.
- **`cmake` tidak dikenali**: Pastikan CMake telah ditambahkan ke PATH sistem operasi Anda.
