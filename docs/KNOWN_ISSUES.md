# 📝 GVM KNOWN ISSUES & COMPILER TECHNICAL NOTES

Dokumen ini mencatat hal teknis dan catatan performa compiler/VM GVM:

---

## 1. JSON Serializer & Deserializer (`std/json.grd`)
* **Status**: JSON Serializer (`encode_int`, `encode_string`, `encode_bool`, `encode_list`, `encode_map`) & Deserializer (`decode_int`, `decode_string`, `decode_bool`, `decode_list`, `decode_map`, `decode`) Murni Bahasa Garuda (`.grd`) terbukti **PASS 100%** di lokal dan live Linux VPS.
* **Catatan**: Unescape `decode_string` pada versi saat ini mendukung `\"` dan `\\`. Karakter kontrol khusus `\n`, `\t`, `\r` direncanakan pada iterasi lanjutan.

## 2. Expression Single-Clause pada Pattern Matching (`tinjau`)
* **Catatan**: Setiap klausa `pattern -> expr` pada ekspresi `tinjau` mengevaluasi 1 ekspresi tunggal. Pernyataan beruntun dalam klausa dihubungkan via pemanggilan fungsi terurut.

## 3. SSH Rate Limiting pada High-Frequency SCP Commands
* **Catatan**: Eksekusi perintah `scp` / `ssh` berkali-kali secara simultan dalam jeda sangat pendek dapat memicu proteksi rate-limiting temporer pada port 22 VPS.
