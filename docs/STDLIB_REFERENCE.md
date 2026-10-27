# 📚 DOKUMENTASI REFERENSI STANDARD LIBRARY (STDLIB) GARUDA

Dokumen referensi API modul pustaka standar Bahasa Garuda (v67.0.0).

---

## 🔤 Modul String (`std/string.grd` / `Str`)

Modul pemrosesan dan manipulasi teks.

| Fungsi | Parameter | Tipe Kembalian | Deskripsi |
| --- | --- | --- | --- |
| `String.panjang(Teks)` | `Teks: String` | `Integer` | Mengembalikan panjang karakter string. |
| `String.besar(Teks)` | `Teks: String` | `String` | Mengubah teks menjadi HURUF BESAR. |
| `String.kecil(Teks)` | `Teks: String` | `String` | Mengubah teks menjadi huruf kecil. |
| `String.potong(Teks, Mulai, Panjang)` | `Teks: String, Mulai: Int, Panjang: Int` | `String` | Memotong string Teks dari indeks Mulai sepanjang Panjang karakter. |
| `String.gabung(ListStr, Pemisah)` | `ListStr: List, Pemisah: String` | `String` | Menggabungkan list string dengan Pemisah. |
| `String.mengandung(Teks, Subteks)` | `Teks: String, Subteks: String` | `Boolean` | Mengembalikan `benar` jika Teks mengandung Subteks. |
| `String.pisah(Teks, Pemisah)` | `Teks: String, Pemisah: String` | `List` | Memisah Teks menjadi list string berdasarkan Pemisah. |
| `String.rapikan(Teks)` | `Teks: String` | `String` | Menghapus whitespace di awal dan akhir Teks. |

---

## 📜 Modul List (`std/list.grd` / `List`)

Modul manipulasi struktur data berurutan (Daftar / List).

| Fungsi | Parameter | Tipe Kembalian | Deskripsi |
| --- | --- | --- | --- |
| `List.panjang(Daftar)` | `Daftar: List` | `Integer` | Mengembalikan jumlah elemen dalam Daftar. |
| `List.tambah(Daftar, Elemen)` | `Daftar: List, Elemen: Any` | `List` | Menambahkan Elemen ke bagian akhir Daftar. |
| `List.kepala(Daftar)` | `Daftar: List` | `Any` | Mengembalikan elemen pertama (head) dari Daftar. |
| `List.ekor(Daftar)` | `Daftar: List` | `List` | Mengembalikan sisa elemen (tail) dari Daftar. |
| `List.peta(Daftar, Fungsi)` | `Daftar: List, Fungsi: Function` | `List` | Menerapkan Fungsi ke setiap elemen Daftar (map). |
| `List.saring(Daftar, Predikat)` | `Daftar: List, Predikat: Function` | `List` | Menyaring elemen Daftar berdasarkan Predikat (filter). |
| `List.lipat(Daftar, Akumulator, Fungsi)` | `Daftar: List, Acc: Any, Fungsi: Function` | `Any` | Mengakumulasikan nilai Daftar dari kiri (fold/reduce). |
| `List.urutkan(Daftar)` | `Daftar: List` | `List` | Mengurutkan elemen Daftar. |
| `List.kosong(Daftar)` | `Daftar: List` | `Boolean` | Mengembalikan `benar` jika Daftar kosong. |

---

## 🗺️ Modul Map (`std/map.grd` / `Map`)

Modul pasangan Kunci-Nilai (Peta / Kamus / Hash Map).

| Fungsi | Parameter | Tipe Kembalian | Deskripsi |
| --- | --- | --- | --- |
| `Map.baru()` | *(tanpa parameter)* | `Map` | Membuat dan mengembalikan Peta/Map baru yang kosong `{}`. |
| `Map.taruh(Peta, Kunci, Nilai)` | `Peta: Map, Kunci: Any, Nilai: Any` | `Map` | Menyimpan atau memperbarui entri Kunci -> Nilai pada Peta. |
| `Map.ambil(Peta, Kunci)` | `Peta: Map, Kunci: Any` | `Option / Any` | Mengambil Nilai dari Peta berdasarkan Kunci. |
| `Map.hapus(Peta, Kunci)` | `Peta: Map, Kunci: Any` | `Map` | Menghapus entri dengan Kunci dari Peta. |
| `Map.panjang(Peta)` | `Peta: Map` | `Integer` | Mengembalikan jumlah entri dalam Peta. |
| `Map.kunci(Peta)` | `Peta: Map` | `List` | Mengembalikan list seluruh Kunci dalam Peta. |
| `Map.nilai(Peta)` | `Peta: Map` | `List` | Mengembalikan list seluruh Nilai dalam Peta. |

---

## 🔄 Modul Enum (`Enum`)

Modul enumerasi dan iterasi data koleksi.

| Fungsi | Parameter | Tipe Kembalian | Deskripsi |
| --- | --- | --- | --- |
| `Enum.masing(Koleksi, Fungsi)` | `Koleksi: List, Fungsi: Function` | `Nil` | Mengeksekusi Fungsi pada setiap elemen Koleksi. |
| `Enum.hitung(Koleksi, Predikat)` | `Koleksi: List, Predikat: Function` | `Integer` | Membilang jumlah elemen yang memenuhi Predikat. |
| `Enum.jumlah(Koleksi)` | `Koleksi: List` | `Integer` | Menjumlahkan seluruh elemen numerik dalam Koleksi. |
