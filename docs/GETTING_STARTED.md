# 🚀 Garuda BEAM — Getting Started Guide

Panduan awal instalasi, kompilasi, dan pembuatan aplikasi terdistribusi pertama Anda dengan **Garuda BEAM (G-BEAM)**.

---

## 🛠️ 1. Prasyarat Sistem

- **Compiler**: GCC 11+, Clang 13+, atau MSVC 2022+ (mendukung C++17)
- **Build System**: CMake 3.18+ & Ninja / Make
- **Libraries**: OpenSSL / mbedTLS & SQLite3 (sudah disertakan dalam vendor)

---

## 📥 2. Clone & Build Repository

```bash
# Clone repository resmi
git clone https://github.com/anggitPM17/garuda.git
cd garuda/garuda_v67_0_production/garuda_v67.0.0

# Build di Linux / macOS
chmod +x scripts/build_linux.sh scripts/run_tests.sh
./scripts/build_linux.sh

# Uji seluruh test suite
./scripts/run_tests.sh
```

---

## 💻 3. Membuat Program "Hello World" Garuda

Buat file `hello.cpp`:

```cpp
#include "garuda/beam/beam.hpp"
#include <iostream>

using namespace garuda::beam;

int main() {
    Runtime rt;
    std::cout << "Halo dari Garuda BEAM v67.0.0! 🦅🇮🇩\n";
    return 0;
}
```

Kompilasi dengan C++17:
```bash
g++ -std=c++17 -Iinclude hello.cpp -o hello -lpthread
./hello
```
