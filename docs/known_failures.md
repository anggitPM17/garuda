# Known Failures

## KF-001: Json.decode_string hang (Test 6b) — TIDAK BERLAKU

**Status:** Tidak berlaku di baseline saat ini (commit 2b74d3e). `std/json.grd` HEAD tidak memiliki `unescape_string_loop` (grep: no matches found). `decode_string` di HEAD adalah versi sederhana (baris 99–106) yang hanya memotong quote luar dan tidak mengandung loop rekursif. KF-001 hanya berlaku untuk versi `std/json.grd` yang ditulis di sesi sebelumnya (sudah di-revert). Jika `decode_string` dengan `unescape_string_loop` di-reintroduce di task terpisah, KF-001 harus diverifikasi ulang.

**Catatan:** Test 6b tidak tercapai di run terakhir karena `CHECK` macro di `tests/test_json_grd.cpp` memanggil `std::exit(1)` saat Test 5b FAIL. Test 6b tidak dapat diverifikasi sampai KF-004 diperbaiki.

## KF-002: Test L2 FAIL — std/json.grd tidak self-contained

**Status:** Open. Masih FAIL di commit 2b74d3e.

**Penyebab:** std/json.grd memakai List.panjang (baris 29, 52, 154) tapi tidak mengimpor std/list.grd. Saat modul user mengimpor std/json.grd, List.panjang tidak resolve.

**Perbaikan:** tambah `impor "std/list.grd"` di awal std/json.grd. Task terpisah.

## KF-003: Test M3 FAIL — import tidak transitive

**Status:** Open. Masih FAIL di commit 2b74d3e.

**Penyebab:** sama dengan KF-002. TestM3 mengimpor std/json.grd dan memakai List.panjang, tapi std/json.grd tidak membawa std/list.grd.

**Perbaikan:** sama dengan KF-002, atau ubah TestM3 agar impor std/list.grd eksplisit.

## KF-004: Test 5b FAIL — encode_map mengasumsikan Map adalah List

**Status:** Open. Masih FAIL di commit 2b74d3e. Juga menyebabkan Test 6b tidak tercapai karena CHECK macro exit(1).

**Penyebab:** Map.baru/Map.taruh mengembalikan Tuple, tapi encode_map di std/json.grd melakukan pattern match [H | T] yang di-compile menjadi opcode List. Opcode List tidak match pada Term(Tuple).

**Perbaikan:** ubah encode_map agar mengakses Map via BIF Map.kunci/Map.ambil atau OpGetTupleElem. Task terpisah.

## KF-005: function_table tidak menyimpan qualified name

**Status:** Open — diidentifikasi saat diagnosis B3a.

**Gejala:** `function_table` hanya menyimpan short name (`decode_int`, `p`, `utama`), tanpa prefix modul (`Json.decode_int`, `TestL2.p`). `match_fn` mengembalikan entry pertama yang cocok, dan karena `function_table` adalah `std::unordered_map`, urutan iterasi tidak dijamin. Jika ada dua fungsi dengan short name yang sama dari modul berbeda, hasil resolusi tidak deterministic.

**Bukti:** Trace `GARUDA_TRACE_FN` untuk TestC: `decode_int -> 260`, `encode_int -> 0`, `utama -> 816`. Tidak ada `Json.decode_int`, `List.panjang`, dll.

**Dampak:** KF-002/003/004 (L2, M3, 5b) tidak bisa ditutup dengan `impor "std/list.grd"` di `std/json.grd` karena name resolution bisa salah.

**Perbaikan (task terpisah):**
1. CodeGenerator harus menyimpan qualified name (`Json.decode_int`) di `function_table`, bukan short name.
2. `match_fn` harus memprioritaskan exact match (qualified) sebelum fallback ke short name.
3. `function_table` sebaiknya `std::map` (ordered) atau `match_fn` harus deterministic (mis. exact match dulu, baru short name dengan prioritas modul yang di-import terakhir).

## KF-006: TestL2 FAIL — name resolution ambigu

**Status:** Open — diidentifikasi saat diagnosis B3a.

**Gejala:** TestL2 (`p(1)` dengan import `std/json.grd`) FAIL. `function_table` berisi `p -> 816` dan `utama -> 827`, tapi `match_fn("p")` mengembalikan entry yang salah (kemungkinan karena `std::unordered_map` iteration order).

**Perbaikan:** sama dengan KF-005.
