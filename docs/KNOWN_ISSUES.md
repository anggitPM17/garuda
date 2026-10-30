# 📝 GVM KNOWN ISSUES & COMPILER TECHNICAL NOTES

Dokumen ini mencatat hal teknis dan catatan performa compiler/VM GVM:

---

## 1. JSON Encoder (`std/json.grd`)
* **Catatan**: JSON Serializer murni dalam `.grd` mendukung `encode_int`, `encode_string`, `encode_bool`, `encode_list`, dan `encode_map`.
* **Status**: Seluruh 5 unit test `garuda_json_grd_tests` terbukti **PASS 100%** di lokal dan VPS. JSON Decoder (`decode`) direncanakan pada iterasi berikutnya setelah penambahan built-in `pisah`, `mengandung`, dan `karakter_ke_int`.

## 2. OpReturn Resolution Priority
* **Catatan**: `OpReturn` pada `script_engine.hpp` memprioritaskan `last_result` jika tidak bernilai `std::monostate`.
* **Status**: Seluruh test regresi TCO, control flow, dan process isolation terbukti **PASS 100%**.

## 3. SSH Rate Limiting pada High-Frequency SCP Commands
* **Catatan**: Eksekusi perintah `scp` / `ssh` berkali-kali secara simultan dalam jeda sangat pendek dapat memicu proteksi rate-limiting temporer pada port 22 VPS.
* **Solusi**: Menjalankan build & unit test secara lokal terlebih dahulu sebelum deploy final ke VPS.
