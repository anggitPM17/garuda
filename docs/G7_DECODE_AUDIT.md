# 📜 AUDIT STRING BUILT-INS & DECODE ROADMAP — G7 FASE A AUDIT REPORT

Dokumen ini mencatat pendaftaran fungsi built-in manipulasi string lanjutan pada **Garuda BEAM** untuk mendukung penguraian string & parser JSON Decoder (`decode`).

---

## 📊 Status Pendaftaran Fungsi String di Runtime GVM

| Fungsi String | Ada di C++ `Str`? | Terdaftar di `handle_builtin`? | Status Teruji |
| :--- | :---: | :---: | :---: |
| **`panjang` (length)** | ✅ Ada | ✅ Ada | ✅ PASS |
| **`besar` (uppercase)** | ✅ Ada | ✅ Ada | ✅ PASS |
| **`kecil` (lowercase)** | ✅ Ada | ✅ Ada | ✅ PASS |
| **`potong` (substring)** | ✅ Ada | ✅ Ada | ✅ PASS |
| **`gabung` (`++`)** | ✅ Ada | ✅ Ada | ✅ PASS |
| **`pisah` (split)** | ✅ Ada | ✅ Terdaftar di G7 | ✅ PASS |
| **`mengandung` (contains)** | ✅ Ada | ✅ Terdaftar di G7 | ✅ PASS |
| **`rapikan` (trim)** | ✅ Ada | ✅ Terdaftar di G7 | ✅ PASS |
| **`karakter_ke_int`** | ✅ Ditambahkan di G7 | ✅ Terdaftar di G7 | ✅ PASS |
| **`int_ke_karakter`** | ✅ Ditambahkan di G7 | ✅ Terdaftar di G7 | ✅ PASS |

---

## 🎯 Dampak terhadap JSON Decoder & Parsing String

1. **`karakter_ke_int("a")` → `97`**: Memungkinkan penguraian karakter individual ASCII untuk mendeteksi digit angka `'0'`..`'9'` (ASCII 48..57) dan karakter kontrol JSON.
2. **`int_ke_karakter(97)` → `"a"`**: Memungkinkan rekonstruksi karakter dari kode integer.
3. **`pisah(S, D)` & `rapikan(S)`**: Memungkinkan tokenisasi string JSON berdasar delimiter koma `,` dan pembersihan whitespace `\n`, `\t`, `\r`.
