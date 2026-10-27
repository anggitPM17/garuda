# 📦 Garuda Static Linking & Single Binary Build Guide

Dokumen ini menjelaskan cara mem-build biner tunggal mandiri (single static binary) untuk Garuda pada Windows (MSVC) dan Linux (GCC/Clang), serta panduan cross-compile.

---

## 🛠️ 1. Static Linking di Windows (MSVC /MT)

Secara default, CMakeLists.txt Garuda mengaktifkan `-DGARUDA_STATIC_LINK=ON`.

### Konfigurasi & Build
```cmd
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DGARUDA_STATIC_LINK=ON
cmake --build build --config Release
```

### Efek `/MT` (MSVC MultiThreaded Static Runtime)
- Runtime C/C++ (`libcmt.lib`, `libcpmt.lib`) di-embed secara langsung ke dalam file `.exe`.
- Hasil biner `garuda_ultra.exe` dapat dijalankan pada mesin Windows mana pun tanpa membutuhkan `VCRUNTIME140.dll` atau `MSVCP140.dll`.

### Keterbatasan DLL Sistem Windows
Dependensi OS Windows standar (`ws2_32.dll` untuk Sockets dan `bcrypt.dll` untuk Cryptographic RNG mbedTLS) disediakan secara dinamis oleh kernel Windows (`C:\Windows\System32\`), sehingga biner tetap ringan dan kompatibel dengan versi Windows 10/11.

---

## 🐧 2. Static Linking di Linux (GCC / Clang)

Pada Linux, opsi `GARUDA_STATIC_LINK=ON` secara otomatis menambahkan flag `-static-libgcc -static-libstdc++`.

### Konfigurasi & Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DGARUDA_STATIC_LINK=ON
cmake --build build --config Release
```

---

## 🌐 3. Cross-Compile ke Linux via CMake Toolchain File

Gunakan file toolchain `cmake/toolchain-linux.cmake` yang sudah disediakan:

```cmd
cmake -B build-linux -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-linux.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux --config Release
```

---

## 🔍 4. Verifikasi Biner Bebas C++ Runtime DLL (Windows)

Gunakan `dumpbin` dari Developer Command Prompt MSVC:
```cmd
dumpbin /dependents build/Release/garuda_ultra.exe
```

Output terverifikasi hanya bergantung pada DLL OS Kernel bawaan:
- `KERNEL32.dll`
- `WS2_32.dll`
- `BCRYPT.dll`
*(Tanpa dependensi `VCRUNTIME140.dll` atau `MSVCP140.dll`).*
