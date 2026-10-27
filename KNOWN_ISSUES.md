# 🐛 Garuda Known Issues & Limitations

## Test Coverage
- **Parser & Codegen**: Terlindungi oleh fuzzing (mini-fuzzer + truncation fuzzing di `test_fuzzing.cpp`) dan error handling terstruktur dengan posisi baris & kolom.
- **Memory**: Arena allocator teruji (`test_memory.cpp` covering 8-byte alignment, reset, capacity overflow handling, dan stub API).
- **Runtime**: Stress test teruji (`test_runtime_stress.cpp` covering parallel spawn lock-free, concurrent messaging, dan preemption CPU adil).

## TCO (Tail Call Optimization)
- **Native / FFI Tail Calls**: TCO saat ini diimplementasikan untuk pemanggilan fungsi bytecode internal Garuda (`OpTailCall`). Pemanggilan ke C/C++ FFI / BIF belum mendukung TCO (akan selalu membutuhkan stack frame native C++).

## Technical Debt
- **Watchdog Timeout**: Watchdog tes C++ berbasis `std::async` + `std::exit(1)` berjalan berdampingan dengan runner `scripts/run_tests.ps1` (process watchdog 30s timeout).
