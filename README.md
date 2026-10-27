# 🦅 Garuda Language & GVM BEAM-Core Runtime v67.0.0

[![Garuda CI](https://github.com/anggitPM17/garuda/actions/workflows/ci.yml/badge.svg)](https://github.com/anggitPM17/garuda/actions/workflows/ci.yml)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![C++ Standard](https://img.shields.io/badge/c%2B%2B-20-blue.svg)]()
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)]()

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

### 3. Membuat Proyek Baru dengan Package Manager `grdmix`
```bash
# Inisialisasi perancangan proyek baru
./build/Release/grdmix new aplikasi_saya
cd aplikasi_saya

# Kompilasi dan jalankan modul utama (src/utama.grd)
../build/Release/grdmix run
```

---

## 📄 Contoh Kode Bahasa Garuda (`.grd`)

- [Contoh Peta & Daftar (`examples/peta_dan_daftar.grd`)](examples/peta_dan_daftar.grd): Demonstrasi pembuatan dan pemrosesan data struktur Map dan List pustaka standar.
- [Contoh Aktor & Supervisor (`examples/aktor_supervisor.grd`)](examples/aktor_supervisor.grd): Demonstrasi spawning aktor (`cakra`), pengiriman pesan (`<-`), dan pemrosesan mailbox (`tampa`).

### 1. Model Aktor Concurrent (PingPong)
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
    ServerPid <- {ping, Dari}
    tampa
        {pong, Sender} ->
            cetak("Diterima balasan PONG dari Server!")
            42
    rampung
rampung
```

### 2. Modul Pustaka Standar Map & List
```garuda
modul PengelolaanData

fungsi utama() ->
    DaftarSiswa = [101, 102, 103, 104]
    TotalSiswa = List.panjang(DaftarSiswa)

    PetaStatus = Map.taruh(Map.baru(), "status", "aktif")
    Status = Map.ambil(PetaStatus, "status")

    cetak("Jumlah siswa terdaftar: " ++ String.panjang(TotalSiswa))
    42
rampung
```

---

## 📚 Dokumentasi Lengkap & Kontribusi

- [Panduan & Tutorial 10 Bab](docs/tutorial.md)
- [Referensi Sintaksis Bahasa](docs/sintaks.md)
- [Dokumentasi Referensi Standard Library (String, List, Map, Enum)](docs/STDLIB_REFERENCE.md)
- [Panduan Penggunaan Package Manager grdmix](docs/GRDMIX.md)
- [Panduan Integrasi C++ Compiler API](docs/COMPILER_API.md)
- [Arsitektur GVM Engine](docs/arsitektur.md)
- [Spesifikasi Model Memori Hibrida GVM](docs/MEMORY_MODEL.md)
- [Desain Arsitektur Stadion Garuda (WASM Playground)](docs/STADION.md)
- [Infrastruktur Terdistribusi](docs/distributed.md)
- [Panduan Kontribusi (CONTRIBUTING.md)](CONTRIBUTING.md)
- [Kebijakan Keamanan (SECURITY.md)](SECURITY.md)

---

## 📜 Lisensi
Dikembangkan di bawah Lisensi [Apache 2.0](LICENSE).
