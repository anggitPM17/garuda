# 🐛 Garuda Known Issues & Limitations

## Test Coverage
- **Parser & Codegen**: Terlindungi oleh fuzzing (mini-fuzzer + truncation fuzzing di `test_fuzzing.cpp`) dan error handling terstruktur dengan posisi baris & kolom.
- **Memory**: Arena allocator teruji (`test_memory.cpp` covering 8-byte alignment, reset, capacity overflow handling, dan stub API).
- **Runtime**: Stress test teruji (`test_runtime_stress.cpp` covering parallel spawn lock-free, concurrent messaging, dan preemption CPU adil).

## TCO (Tail Call Optimization)
- **Native / FFI Tail Calls**: TCO saat ini diimplementasikan untuk pemanggilan fungsi bytecode internal Garuda (`OpTailCall`). Pemanggilan ke C/C++ FFI / BIF belum mendukung TCO (akan selalu membutuhkan stack frame native C++).

## Technical Debt
- **CI Timeout**: Runner `scripts/run_tests.ps1` menggunakan proses terpisah dengan timeout 30 detik. Namun, jika ada tes yang hang di dalam binary C++ (misal infinite loop di parser), kita harus menunggu 30 detik. Ke depannya, kita bisa menambahkan timeout per-test di dalam kode C++.
