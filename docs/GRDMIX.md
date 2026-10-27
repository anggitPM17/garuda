# 📦 PANDUAN PENGGUNAAN PACKAGE MANAGER GARUDA (`grdmix`)

Dokumen spesifikasi dan panduan penggunaan perkakas CLI Package Manager `grdmix` untuk Bahasa Garuda.

---

## 🎯 1. Apa itu `grdmix`?

`grdmix` adalah perkakas otomatisasi build, package manager, dan peluncur tes (*test runner*) resmi untuk Bahasa Garuda, terinspirasi oleh `mix` di ekosistem Elixir dan `cargo` di ekosistem Rust.

`grdmix` mengotomatiskan seluruh alur pengembangan aplikasi Garuda:
- Perancangan proyek baru (*scaffolding*)
- Manajemen berkas konfigurasi (`grdmix.toml`)
- Kompilasi biner bytecode (`.gvm`)
- Eksekusi runtime modul utama
- Pelaksanaan pengujian unit & integrasi otomatis (`test/`)

---

## 🚀 2. Membuat Proyek Baru

### A. Membuat Folder Proyek Baru (`grdmix new`)
```bash
grdmix new aplikasi_saya
cd aplikasi_saya
```

### B. Inisialisasi di Folder Saat Ini (`grdmix init`)
```bash
mkdir proyek_baru
cd proyek_baru
grdmix init
```

---

## 📁 3. Struktur Folder Proyek

Perintah `grdmix new` atau `grdmix init` akan menghasilkan struktur direktori standar:

```text
aplikasi_saya/
├── grdmix.toml         # Konfigurasi proyek & daftar dependensi
├── README.md           # Dokumentasi proyek
├── .gitignore          # Konfigurasi eksklusi Git (*.gvm, build/)
├── src/
│   └── utama.grd       # Kode sumber modul utama
└── test/
    └── test_utama.grd  # Berkas pengujian unit
```

### Format File `grdmix.toml`
```toml
[project]
name = "aplikasi_saya"
version = "0.1.0"
author = "Pengembang Garuda"
description = "Proyek Garuda pertama saya"

[dependencies]
```

---

## 🛠️ 4. Kompilasi & Eksekusi

### Mengompilasi Kode Sumber (`grdmix build`)
Mengompilasi seluruh berkas `.grd` di folder `src/` menjadi biner bytecode `.gvm`:
```bash
grdmix build
# atau: grdmix compile
```

### Menjalankan Proyek (`grdmix run`)
Mengompilasi dan mengeksekusi fungsi `utama()` di berkas `src/utama.grd`:
```bash
grdmix run
```

---

## 🧪 5. Menjalankan Pengujian Otomatis (`grdmix test`)

Mengeksekusi seluruh berkas pengujian `.grd` yang berada di direktori `test/`:
```bash
grdmix test
```

Contoh output:
```text
  ✓ Test test_utama.grd PASS
✅ Pengujian Selesai: 1/1 PASS
```
