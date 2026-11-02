# Panduan Kontribusi Bahasa Garuda

Terima kasih atas minat Anda untuk berkontribusi pada pengembangan **Bahasa Garuda v67.0.0**!

---

## 1. Persyaratan Pengembang

- Compiler C++20 (MSVC 2022 / GCC 10+ / Clang 12+)
- CMake 3.20+
- Node.js 18+ (untuk LSP & Ekstensi VS Code)

---

## 2. Langkah Kompilasi Proyek

```powershell
# 1. Konfigurasi CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Kompilasi CLI Tooling grd
cmake --build build --config Release --target grd
```

---

## 3. Menjalankan Suite Pengujian

Seluruh pengujian unit C++ wajib **PASS 100%** sebelum mengajukan Pull Request:

```powershell
.\build\Release\grd.exe test
```

Target status pengujian: `14 PASS | 0 FAIL | 0 SKIP`.

---

## 4. Cara Menambahkan Fungsi BIF / Modul Baru

1. **Implementasi BIF C++**:
   Tambahkan logika handler pada `handle_builtin` di `include/garuda/beam/tools/script_engine.hpp`.
2. **Pendaftaran Nama BIF**:
   Daftarkan nama fungsi pada lambda `is_bif_name` di `include/garuda/compiler/codegen.hpp`.
3. **Pustaka Standar Garuda**:
   Daftarkan wrapper murni fungsi pada berkas modul terkait di `std/*.grd`.
4. **Unit Test**:
   Tambahkan pengujian unit C++ di direktori `tests/` dan daftarkan executable pada `CMakeLists.txt` serta `src/grd.cpp`.

---

## 5. Pedoman Kode (*Code Style*)

- Gunakan penamaan BIF C++ yang konsisten dengan pustaka `std/*.grd`.
- Bahasa pengantar dokumentasi dan nama modul pustaka standar menggunakan Bahasa Indonesia resmi (*modul*, *fungsi*, *rampung*, *yen*, *lain*, *nalika*).
- Dilarang keras mengubah bagian register allocation / tail call tanpa menyertakan bukti regresi lulus 100%.
