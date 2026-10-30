# 📝 GVM KNOWN ISSUES & COMPILER TECHNICAL NOTES

Dokumen ini mencatat hal teknis dan catatan performa compiler/VM GVM:

---

## 1. OpReturn Resolution Priority
* **Catatan**: `OpReturn` pada `script_engine.hpp` memprioritaskan `last_result` jika tidak bernilai `std::monostate`.
* **Status**: Seluruh test regresi TCO, control flow, dan process isolation terbukti **PASS 100%**.

## 2. Codegen Pattern Matching (`tinjau` / `case`)
* **Catatan**: Pattern matching `tinjau ... ing` dalam AST `case` saat ini dalam tahap penyempurnaan di level `codegen.hpp` (Test 6 pada control flow suite dilewati/skipped).
* **Status**: Direncana disempurnakan pada **Fase G2** sebelum memicu modul `std/result.grd` dan `std/json.grd`.

## 3. SSH Rate Limiting pada High-Frequency SCP Commands
* **Catatan**: Eksekusi perintah `scp` / `ssh` berkali-kali secara simultan dalam jeda sangat pendek dapat memicu proteksi rate-limiting temporer pada port 22 VPS.
* **Solusi**: Menjalankan build & unit test secara lokal terlebih dahulu sebelum deploy final ke VPS.
