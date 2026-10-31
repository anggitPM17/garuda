# Known Failures

## KF-001: Json.decode_string hang (Test 6b)

**Status:** Tidak diverifikasi di commit ba79947. Kemungkinan diperbaiki oleh 8266901 (protect_param_regs di gen_call/gen_tail_call), tapi Test 6b tidak tercapai di run terakhir (berhenti di Test 5b).

**Tindakan:** Jalankan garuda_json_grd_tests.exe sampai selesai (atau sampai Test 6b jika ada di std/json.grd saat ini) untuk verifikasi. Jika 6b PASS, tutup KF-001. Jika 6b hang, KF-001 masih open.

## KF-002: Test L2 FAIL — std/json.grd tidak self-contained

**Status:** Open. Masih FAIL di commit ba79947.

**Penyebab:** std/json.grd memakai List.panjang (baris 29, 52, 154) tapi tidak mengimpor std/list.grd. 8266901 tidak mengubah std/json.grd, jadi bug ini tetap ada.

**Perbaikan:** tambah `impor "std/list.grd"` di awal std/json.grd. Task terpisah.

## KF-003: Test M3 FAIL — import tidak transitive

**Status:** Open. Masih FAIL di commit ba79947.

**Penyebab:** sama dengan KF-002. TestM3 mengimpor std/json.grd dan memakai List.panjang, tapi std/json.grd tidak membawa std/list.grd.

**Perbaikan:** sama dengan KF-002, atau ubah TestM3 agar impor std/list.grd eksplisit.

## KF-004: Test 5b FAIL — encode_map mengasumsikan Map adalah List

**Status:** Open. Masih FAIL di commit ba79947.

**Penyebab:** Map.baru/Map.taruh mengembalikan Tuple, tapi encode_map di std/json.grd melakukan pattern match [H | T] yang di-compile menjadi opcode List. 8266901 tidak mengubah std/json.grd, jadi bug ini tetap ada.

**Perbaikan:** ubah encode_map agar mengakses Map via BIF Map.kunci/Map.ambil atau OpGetTupleElem. Task terpisah.
