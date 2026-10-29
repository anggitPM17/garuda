# 🔌 Garuda BEAM — C-ABI Native Interop Guide (FASE D.2)

Dokumen ini menjelaskan arsitektur dan keunggulan **C-ABI Native Interop / Foreign Function Interface (FFI)** pada Garuda BEAM.

---

## 🌟 Keunggulan C-ABI FFI Garuda BEAM vs Erlang NIF

| Fitur / Karakteristik | Erlang/OTP (NIF) | Garuda BEAM (C-ABI Native Interop) |
|---|---|---|
| **Kemudahan Integrasi** | Perlu NIF wrapper & C marshalling | Langsung panggil via C++ header-only |
| **Crash Safety** | Bug NIF dapat merobohkan seluruh VM | Exception ditangkap aman tanpa crash VM |
| **Memori Management** | Manual `enif_alloc` / `enif_free` | **RAII (std::unique_ptr, std::shared_ptr)** |
| **Data Overhead** | Copy & Unmarshal | **Zero-Copy (`std::span`, `std::string_view`)** |
| **Sistem Build** | Rebar3 + Makefile terpisah | **Satu CMake unified build system** |

---

## 🏗️ Komponen Utama C-ABI FFI Engine

1. **`CFunction` & `DlHandle` (`include/garuda/ffi/ffi.hpp`)**:
   - Abstraksi `dlopen`/`dlsym` (POSIX Linux) & `LoadLibrary`/`GetProcAddress` (Windows).
   - Template method `call<Ret, Args...>()` type-safe tanpa marshalling overhead.

2. **`CRegistry` (`include/garuda/ffi/registry.hpp`)**:
   - Pendaftaran alias fungsi C dari pustaka bersama (`.so` / `.dll`).

3. **`CurlBinding` & `ZlibBinding` (`include/garuda/ffi/native_binding.hpp`)**:
   - Binding toleran & fallback otomatis untuk pustaka populer seperti `libcurl` dan `zlib`.

4. **DSL `panggil_c` (`include/garuda/ffi/dsl.hpp`)**:
   - Kata kunci skrip Garuda untuk mengeksekusi fungsi C langsung dari skrip `.grd`:
   ```garuda
   panggil_c get_pid
   ```
