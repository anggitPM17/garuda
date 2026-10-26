# 🐛 Garuda Known Issues & Limitations

## Catatan Historis: Control Flow Cacat (sebelum 26 Okt 2026)

Sebelum 26 Okt 2026, `OpJump` (12) dan `OpJumpIfFalse` (25) tidak ditangani di `execute_slice` (interpreter.hpp). Akibatnya:
- Semua `yen/lain`, `cond`, `case` mengeksekusi cabang then DAN else secara berurutan, bukan bersyarat.
- Tes yang lulus sebelum tanggal tersebut TIDAK menjamin kebenaran control flow.
- Klaim performa di REMEDIATION_PLAN.md sebelum tanggal tersebut diukur dengan control flow yang cacat. Angka HTTP telah diukur ulang (lihat REMEDIATION_PLAN.md §2.2).

Perbaikan: commit 26 Okt 2026 menambahkan handler OpJump dan OpJumpIfFalse. Tes control flow eksplisit ditambahkan di `tests/test_control_flow.cpp`. Mutation test membuktikan tes tersebut bermakna (gagal saat OpJumpIfFalse dihapus).

## TCO (Tail Call Optimization)
- **Native / FFI Tail Calls**: TCO saat ini diimplementasikan untuk pemanggilan fungsi bytecode internal Garuda (`OpTailCall`). Pemanggilan ke C/C++ FFI / BIF belum mendukung TCO (akan selalu membutuhkan stack frame native C++).
