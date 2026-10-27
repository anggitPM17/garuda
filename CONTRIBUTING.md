# 🤝 Panduan Kontribusi Garuda (Contributing Guidelines)

Terima kasih telah tertarik untuk berkontribusi pada pengembangan Bahasa Garuda!

## 🛠️ 1. Persiapan & Build Lokal

### Persyaratan System
- Windows 10/11 dengan **Visual Studio 2022** (Workload C++ Desktop Development) atau Build Tools for Visual Studio.
- **CMake** version 3.14 atau yang lebih baru.
- **Ninja** (opsional, tetapi direkomendasikan).

### Cara Build
```powershell
# Clone repositori
git clone https://github.com/anggitPM17/garuda.git
cd garuda

# Konfigurasi CMake
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# Build executable
cmake --build build
```

---

## 🧪 2. Menjalankan Tes

Sebelum membuat Pull Request, pastikan seluruh test suite lokal lulus menggunakan script pembantu PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run_tests.ps1
```

Script ini akan mem-build dan mengeksekusi:
- `garuda_tco_tests` (Tail Call Optimization)
- `garuda_control_flow_tests` (Percabangan & Control Flow)
- `garuda_isolation_tests` (Isolasi Proses Aktor)
- `garuda_fault_tests` (Fault Tolerance & Fault Isolation)

---

## 📋 3. Aturan Pull Request (PR)

1. **Wajib Lulus CI**: Seluruh Pull Request harus lulus pemeriksaan otomatis di GitHub Actions CI.
2. **Wajib Ada Tes**: Setiap fitur baru, perbaikan bug, atau refactoring WAJIB disertai tes unit/integrasi otomatis.
3. **Commit Pesan Jelas**: Gunakan format commit yang terstruktur (contoh: `feat: ...`, `fix: ...`, `docs: ...`, `test: ...`).
4. **Prinsip Lapis 0**: Utamakan pengerasan fondasi dan stabilitas compiler/runtime.

---

## 🎨 4. Gaya Penulisan Kode (Code Style)

- **Standar Bahasa**: C++20 (`cxx_std_20`).
- **Penamaan Class & Struct**: `PascalCase` (contoh: `CodeGenerator`, `GvmProcessState`).
- **Penamaan Fungsi & Variabel**: `snake_case` (contoh: `execute_slice`, `alloc_reg`).
- **Header Guard**: Gunakan `#pragma once`.
- **Manajemen Memori**: Hindari alokasi raw pointer manual; gunakan `std::unique_ptr` / `std::shared_ptr` atau alokator pool bawaan runtime.
