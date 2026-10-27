# 🤝 Panduan Kontribusi Garuda

Terima kasih telah tertarik untuk berkontribusi pada proyek **Garuda**! Proyek ini dikembangkan dengan komitmen tinggi terhadap kualitas kode, determinisme build, dan stabilitas performa.

---

## 🚀 1. Cara Memulai

### Prasyarat
- **Compiler**: MSVC (Visual Studio 2022/2026), GCC 11+, atau Clang 13+ dengan dukungan C++20.
- **Build System**: CMake 3.14+ dan Ninja / MSBuild.
- **PowerShell**: Untuk menjalankan skrip pengujian lokal.

### Langkah Build & Tes Lokal
```bash
# 1. Clone repositori
git clone https://github.com/richat/garuda.git
cd garuda

# 2. Konfigurasi CMake
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# 3. Jalankan seluruh test suite dengan skrip watchdog
powershell -ExecutionPolicy Bypass -File scripts/run_tests.ps1
```

---

## 📝 2. Aturan Commit & Pull Request

1. **Satu Fitur / Bugfix Per PR**: Buat Pull Request yang berfokus pada satu tujuan spesifik.
2. **Wajib Menyertakan Tes**: Setiap perubahan atau penambahan fitur WAJIB disertai dengan unit test atau E2E test di folder `tests/`.
3. **Lulus 100% CI/CD**: PR hanya akan digabungkan (merge) jika seluruh test suite lulus di runner lokal dan GitHub Actions CI.
4. **Disiplin Pesan Commit**: Gunakan pesan commit yang deskriptif dalam Bahasa Indonesia atau Inggris, contoh:
   - `feat(io): tambah modul rate_limiter`
   - `fix(compiler): perbaiki penanganan error sintaks pada parser`

---

## 🎨 3. Standar Kode & Style Guide

- **Bahasa**: C++20 murni tanpa dependensi eksternal yang tidak perlu.
- **Penamaan**:
  - Class / Struct: `PascalCase` (contoh: `HealthChecker`, `QueryBuilder`).
  - Function / Method / Variable: `snake_case` (contoh: `get_secret`, `is_allowed`).
  - Constant / Enum: `PascalCase` atau `UPPER_CASE`.
- **Formatting**: Gunakan indentasi 4 spasi (bukan tab) dan pastikan tidak ada whitespace ekstra di akhir baris.

---

## 💬 4. Diskusi & Komunikasi

- **Bug & Permintaan Fitur**: Gunakan [GitHub Issues](https://github.com/richat/garuda/issues) dengan template yang disediakan.
- **Tanya Jawab & Ide**: Gunakan [GitHub Discussions](https://github.com/richat/garuda/discussions).
