# 🦅 Garuda Language & GVM BEAM-Core Runtime v67.0.0

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![C++ Standard](https://img.shields.io/badge/c%2B%2B-20-blue.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)]()

**Garuda** adalah Bahasa Pemrograman Concurrent Actor-Model berkinerja tinggi yang berjalan di atas **GVM (Garuda Virtual Machine)** berbasis arsitektur BEAM Erlang/OTP.

---

## ⚡ Fitur Utama

- 🇮🇩 **Sintaks Dwibahasa Nusantara & Elixir**: Tulis kode dengan kata kunci lokal (`modul`, `fungsi`, `rampung`, `cakra`, `tampa`, `yen`) atau Elixir.
- ⚡ **Tail Call Optimization (TCO)**: Rekursi ekor 10.000.000+ iterasi dengan penggunaan *stack memory* tetap $O(1)$.
- 🐣 **Million-Scale Concurrency**: Sanggup mengeksekusi **1.000.000+ Proses Aktor Konkuren** secara paralel dengan RAM < 2.5 GB.
- ⏱️ **Preemptive Scheduler**: Anggaran 4000 *reductions per slice* mencegah *CPU starvation*.
- 🧹 **Per-Process Generational GC**: Pembersihan memori lokal per-proses tanpa *global stop-the-world pause* (Jeda GC < 10 ms).
- 📦 **Binary Bytecode Format (`.gvm`)**: Kompilasi biner terstruktur dengan verifikasi *FNV1a Checksum*.
- 🗳️ **Raft Consensus Protocol**: Pemilihan *Leader* otomatis & replikasi log konsensus terdistribusi.
- 🌐 **Global Process Registry & Distributed PubSub**: Pencarian PID global & *broadcast* pesan masif lintas-server VPS via Soket TCP.
- 🔥 **Hot Code Reloading**: Pembaruan modul biner di memori secara *live* tanpa *downtime* (*Zero Downtime Upgrade*).

---

## 🚀 Panduan Cepat (*Quick Start*)

### 1. Kompilasi Engine dengan CMake
```bash
cmake -S . -B build
cmake --build build --config Release
```

### 2. Jalankan Program Contoh
```bash
# Menjalankan kode sumber .grd langsung
./build/Release/grdc run examples/halo.grd
./build/Release/grdc run examples/faktorial.grd
./build/Release/grdc run examples/pingpong.grd

# Kompilasi ke biner .gvm dan jalankan
./build/Release/grdc compile examples/halo.grd
./build/Release/grdc run examples/halo.gvm
```

---

## 📄 Contoh Kode Bahasa Garuda (`.grd`)

```garuda
modul PingPong

fungsi server() ->
    tampa
        {ping, Dari} ->
            Dari <- {pong, diri}
    rampung
rampung

fungsi utama() ->
    ServerPid = cakra(fungsi() -> server() rampung)
    ServerPid <- {ping, diri}
    tampa
        {pong, Sender} ->
            cetak("Diterima balasan PONG dari Server!")
            42
    rampung
rampung
```

---

## 📚 Dokumentasi Lengkap

- [Panduan & Tutorial 10 Bab](docs/tutorial.md)
- [Referensi Sintaksis Bahasa](docs/sintaks.md)
- [Arsitektur GVM Engine](docs/arsitektur.md)
- [Infrastruktur Terdistribusi](docs/distributed.md)

---

## 📜 Lisensi
Dikembangkan di bawah Lisensi [MIT](LICENSE).
