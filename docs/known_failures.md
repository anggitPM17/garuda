# Known Failures

Tidak ada known failure saat ini. Semua KF-001 s/d KF-006 telah ditutup.

## Riwayat

- KF-001 (Json.decode_string hang) — Tidak berlaku. `std/json.grd` HEAD tidak memiliki `unescape_string_loop`. Jika `decode_string` dengan `unescape_string_loop` di-reintroduce, verifikasi ulang.
- KF-002 (Test L2) — Selesai di commit `df6800e`. `function_table` menyimpan qualified name, `match_fn` deterministic, `merge_output` menggeser `OpCall`/`OpTailCall`/`OpSpawn` literal IP.
- KF-003 (Test M3) — Selesai di commit `df6800e`. `std/json.grd` mengimpor `std/list.grd`, import transitive bekerja.
- KF-004 (Test 5b) — Selesai di commit `df9ddaf`. BIF `Map.kunci` ditambahkan, `encode_map` menggunakan `Map.kunci`/`Map.ambil`.
- KF-005 (function_table tidak menyimpan qualified name) — Selesai di commit `697d986`. `codegen.hpp` menyimpan `module_name + "." + fn->name()` di `function_table`.
- KF-006 (match_fn tidak deterministic) — Selesai di commit `697d986`. `match_fn` memprioritaskan exact match, fallback ke short name dengan offset terkecil.

## Status Test Suite

- `garuda_isolate_binop_tests.exe`: 26/26 PASS (A–N3).
- `garuda_json_grd_tests.exe`: 6/6 PASS (Test 1–5b).