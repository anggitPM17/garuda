# 📜 CATATAN PERUBAHAN (CHANGELOG)

Seluruh perubahan penting pada proyek Bahasa Garuda dicatat dalam berkas ini.

Format berkas ini mengacu pada [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
dan proyek ini mematuhi [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Ditambahkan
- **Hybrid Memory Model**: Arsitektur memori hibrida menggabungkan *Copying GC per-actor* dan *Arena Allocator* deterministik $O(1)$ untuk hot path (`docs/MEMORY_MODEL.md` & `include/garuda/beam/core/memory.hpp`).
- **Standard Library Inti**: Modul `String` (`std/string.grd`), `List` (`std/list.grd`), `Map` (`std/map.grd`), `Enum`, dan `IO` dengan C++ BIF helpers.
- **Package Manager `grdmix`**: CLI perancangan proyek baru (`grdmix new`, `grdmix init`), kompilasi (`grdmix build`), eksekusi (`grdmix run`), dan peluncur tes otomatis (`grdmix test`).
- **Embeddable C++ Compiler API**: Pustaka C++ header-only terstruktur (`Lexer` -> `Parser` -> `CodeGenerator`) yang dapat disematkan langsung ke aplikasi C++ eksternal (`docs/COMPILER_API.md`).
- **Formatter & Linter Engine**: Otomatisasi format kode 4-spasi dan analisis statis kode (*snake_case naming*, *unused variables*).
- **Deterministic Build System**: Opsi kompilasi MSVC `/Brepro` dan skrip otomatisasi `check_deterministic.ps1` untuk menjamin reproduksibilitas binary 100%.
- **GitHub Actions CI/CD**: Workflow otomatisasi CI berbasis Windows MSVC + Ninja dengan pengujian deterministik dan pencegahan regresi performa.
- **Benchmark & Regression Prevention Suite**: Skrip pengujian otomatis `run_benchmarks.ps1` untuk throughput HTTP Server (baseline 16.300 req/s) dan Actor Spawn Throughput (baseline 638.849 processes/s).

### Diperbaiki
- **Control Flow Correction**: Implementasi handler `OpJump` dan `OpJumpIfFalse` di interpreter GVM untuk menjamin kebenaran percabangan `yen/lain` bersarang dan else-if chain.
- **Tail Call Optimization (TCO)**: Deteksi tail call terstruktur pada ekspresi cabang `yen/lain` dengan perlindungan register untuk menjamin alokasi stack $O(1)$.
- **Compiler Warnings**: Pembersihan seluruh compiler warning C4189 pada MSVC.

---

## [0.1.0] - 2026-10-26

### Ditambahkan
- **Lapis 0 Fondasi Garuda Engine**:
  - GVM (Garuda Virtual Machine) register-based interpreter berkecepatan tinggi berbasis arsitektur BEAM Erlang/OTP.
  - Sintaks Dwibahasa Nusantara (`modul`, `fungsi`, `rampung`, `cakra`, `tampa`, `yen`) dan Elixir (`defmodule`, `def`, `end`, `spawn`, `receive`, `if`).
  - Preemptive Scheduler dengan kuota 4.000 *reductions per slice*.
  - Protocol Konsensus Raft, Distributed PubSub, dan Global Process Registry.
  - Hot Code Reloading biner `.gvm`.
