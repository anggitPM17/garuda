# Hasil Benchmark Performa Bahasa Garuda (v67.0.0)

**Tanggal**: November 2026  
**Environment**: Windows 10 (Build 19045), MSVC 2022 (v18.11), x64  
**Build Target**: Release (`-DCMAKE_BUILD_TYPE=Release`)  
**Metode Pengukuran**: 3 iterasi eksekusi `std::chrono::high_resolution_clock`, diurutkan untuk mengambil nilai Median.

---

## 📊 Hasil Pengukuran Performa

| Kategori Benchmark | Deskripsi Pengujian | Median (ms) | Min (ms) | Max (ms) |
| :--- | :--- | :---: | :---: | :---: |
| **Startup Engine** | Evaluation & execution `hello_world` | **0.032 ms** | 0.024 ms | 0.360 ms |
| **Actor Concurrency** | `cakra` spawn process + `kirim` pesan | **0.071 ms** | 0.067 ms | 0.271 ms |
| **Map Operations** | `Map.taruh` multi-entry + `Map.ambil` | **1.086 ms** | 0.935 ms | 1.216 ms |
| **String Manipulation** | `String.ganti` + `String.balik` (17 std/string) | **1.538 ms** | 1.298 ms | 1.543 ms |
| **Functional List** | `List.peta` dengan lambda anonim | **4.236 ms** | 4.076 ms | 4.478 ms |
| **JSON Serialization** | `Json.encode_list` + `Json.decode_list` | **7.661 ms** | 7.078 ms | 11.251 ms |

---

## ⚡ Analisis Performa

1. **Waktu Startup Instan (0.032 ms)**: GVM dapat membangkitkan instance interpreter dan mengeksekusi skrip dalam waktu 32 mikrodetik, sangat ideal untuk CLI tooling dan AWS Lambda / Serverless container.
2. **Actor Concurrency Ultra-Rendah (0.071 ms)**: Alokasi memori Actor PID dan pengiriman pesan mailbox membutuhkan waktu 71 mikrodetik, membuktikan efisiensi Actor Model GVM.
3. **Optimasi Register-based Bytecode**: Eksekusi `List.peta` dan `String` murni yang teroptimasi tanpa alokasi objek berat C++.

---

## 🛠️ Cara Menjalankan Ulang Benchmark

```powershell
cmake --build build --target garuda_benchmark_runner --config Release
.\build\Release\garuda_benchmark_runner.exe
```
