# 📜 AUDIT STRING MANIPULATION & DESAIN `std/json.grd` — G5 FASE A AUDIT REPORT

Dokumen ini mencatat hasil audit ketersediaan operasi manipulasi string untuk implementasi **JSON Encoder/Decoder** pada bahasa Garuda (`.grd`).

---

## 📊 Hasil Audit Fungsi Manipulasi String

| Fungsi String | Ada di C++ `Str`? | Terdaftar di `handle_builtin`? | Dapat Dipakai di `.grd`? | Lokasi Kode C++ |
| :--- | :---: | :---: | :---: | :--- |
| **`panjang` (length)** | ✅ Ada | ✅ Ada | ✅ Ya | `include/garuda/beam/stdlib/string.hpp:18` |
| **`besar` (uppercase)** | ✅ Ada | ✅ Ada | ✅ Ya | `include/garuda/beam/stdlib/string.hpp:35` |
| **`kecil` (lowercase)** | ✅ Ada | ✅ Ada | ✅ Ya | `include/garuda/beam/stdlib/string.hpp:41` |
| **`potong` (substring)** | ✅ Ada | ✅ Ada | ✅ Ya | `include/garuda/beam/stdlib/string.hpp:30` |
| **`gabung` (`++`)** | ✅ Ada | ✅ Ada | ✅ Ya | `include/garuda/beam/stdlib/string.hpp:22` / `OpConcat` |
| **`pisah` (split)** | ✅ Ada | ❌ Belum | ❌ Belum | `include/garuda/beam/stdlib/string.hpp:51` |
| **`mengandung` (contains)** | ✅ Ada | ❌ Belum | ❌ Belum | `include/garuda/beam/stdlib/string.hpp:47` |
| **`rapikan` (trim)** | ✅ Ada | ❌ Belum | ❌ Belum | `include/garuda/beam/stdlib/string.hpp:61` |
| **`karakter_ke_int`** | ❌ Belum | ❌ Belum | ❌ Belum | Belum diimplementasikan |
| **`int_ke_karakter`** | ❌ Belum | ❌ Belum | ❌ Belum | Belum diimplementasikan |

---

## 🎯 Usulan Scope Minimal G5 (Fase B)

### 1. **JSON Encoding (`encode`) (Murni `.grd`)**
* `std/json.grd` mengimplementasikan serialisasi data ke string JSON murni dalam `.grd`:
  * `encode_int(N)` → `"42"`
  * `encode_string(S)` → `"\"halo\""`
  * `encode_bool(B)` → `"true"` / `"false"`
  * `encode_list(L)` → `"[1, 2, 3]"`
  * `encode_map(M)` → `"{ \"a\": 1 }"`
* **Dukungan Primitif**: Operasi penggabungan string `++` dan pattern matching tuple/list `[H | T]` yang sudah 100% teruji di G1–G4 sangat ideal untuk JSON Encoder.

### 2. **JSON Decoding (`decode`)**
* Parsing JSON string kompleks membutuhkan penguraian karakter demi karakter (`karakter_ke_int`) atau `pisah()` / `mengandung()`.
* **Rekomendasi**: Pada G5, kita selesaikan **JSON Encoder Murni `.grd`** secara sempurna dan komprehensif. Pada G6, kita daftarkan `pisah`, `mengandung`, `rapikan`, dan `karakter_ke_int` ke dalam `handle_builtin` untuk memicu JSON Decoder murni `.grd`.
