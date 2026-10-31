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
