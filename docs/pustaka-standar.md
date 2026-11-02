# Referensi Pustaka Standar Bahasa Garuda

Dokumentasi lengkap modul pustaka standar Bahasa Garuda (`std/*.grd`).

---

## 1. Modul `std/list.grd`

Pustaka operasi pada struktur data List.

| Fungsi | Signature | Deskripsi |
| :--- | :--- | :--- |
| `List.panjang` | `(List) -> Integer` | Mengembalikan jumlah elemen dalam list |
| `List.kepala` | `(List) -> Any` | Mengembalikan elemen pertama list |
| `List.ekor` | `(List) -> List` | Mengembalikan sisa list setelah elemen pertama |
| `List.tambah` | `(List, Any) -> List` | Menambahkan elemen ke akhir list |
| `List.balik` | `(List) -> List` | Membalikkan urutan elemen list |
| `List.anggota` | `(List, Any) -> Boolean` | Memeriksa apakah X merupakan anggota list |
| `List.peta` | `(List, Function) -> List` | Mentransformasikan setiap elemen list dengan fungsi lambda |
| `List.saring` | `(List, Function) -> List` | Menyaring elemen list berdasarkan predikat fungsi lambda |
| `List.lipat` | `(List, Init, Function) -> Any` | Menjalankan akumulasi reduce/fold pada elemen list |

```garuda
impor "std/list.grd"
List.peta([1, 2, 3], fungsi(X) -> X * 2 rampung)  -- Mengembalikan [2, 4, 6]
```

---

## 2. Modul `std/string.grd` (17 Fungsi)

Pustaka manipulasi string murni.

| Fungsi | Signature | Deskripsi |
| :--- | :--- | :--- |
| `String.panjang` | `(String) -> Integer` | Mengembalikan panjang karakter string |
| `String.besar` | `(String) -> String` | Mengubah string menjadi huruf besar |
| `String.kecil` | `(String) -> String` | Mengubah string menjadi huruf kecil |
| `String.potong` | `(String, Start, Len) -> String` | Mengambil substring dari Mula sebanyak Panjang |
| `String.gabung` | `(String, String) -> String` | Penggabungan dua string (setara `++`) |
| `String.pisah` | `(String, Delim) -> List` | Memecah string menjadi List berdasarkan pembatas |
| `String.mengandung`| `(String, Sub) -> Boolean` | Memeriksa apakah string mengandung substring Sub |
| `String.rapikan` | `(String) -> String` | Menghapus whitespace di awal dan akhir string |
| `String.ganti` | `(String, From, To) -> String` | Mengganti seluruh kemunculan From dengan To |
| `String.cari` | `(String, Sub) -> Integer` | Indeks pertama substring Sub, atau -1 jika tidak ada |
| `String.mulai_dengan`| `(String, Prefix) -> Boolean` | Memeriksa apakah string diawali dengan Prefix |
| `String.akhir_dengan`| `(String, Suffix) -> Boolean` | Memeriksa apakah string diakhiri dengan Suffix |
| `String.balik` | `(String) -> String` | Membalikkan urutan karakter string |
| `String.ulang` | `(String, N) -> String` | Mengulangi string N kali |
| `String.ke_besar_awal`| `(String) -> String` | Mengubah huruf pertama menjadi huruf besar |
| `String.karakter_ke_int`| `(String) -> Integer` | Mengembalikan ASCII/UTF code karakter pertama |
| `String.int_ke_karakter`| `(Integer) -> String` | Mengonversi ASCII code menjadi string karakter |

---

## 3. Modul `std/map.grd` (10 Fungsi)

Pustaka manipulasi key-value Map (direpresentasikan secara teroptimasi sebagai Tuple entri `{K, V}`).

| Fungsi | Signature | Deskripsi |
| :--- | :--- | :--- |
| `Map.baru` | `() -> Map` | Membuat Map baru yang kosong |
| `Map.taruh` | `(Map, Kunci, Nilai) -> Map` | Menambahkan atau memperbarui entri pada Map |
| `Map.ambil` | `(Map, Kunci) -> Any` | Mengambil nilai berdasarkan Kunci dari Map |
| `Map.hapus` | `(Map, Kunci) -> Map` | Menghapus entri Kunci dari Map |
| `Map.kunci` | `(Map) -> List` | Mengembalikan List seluruh kunci di Map |
| `Map.nilai` | `(Map) -> List` | Mengembalikan List seluruh nilai di Map |
| `Map.ukuran` | `(Map) -> Integer` | Mengembalikan jumlah entri pada Map |
| `Map.mengandung_kunci`| `(Map, Kunci) -> Boolean` | Memeriksa keberadaan Kunci di Map |
| `Map.kosong` | `(Map) -> Boolean` | Memeriksa apakah Map kosong |
| `Map.gabung` | `(M1, M2) -> Map` | Menggabungkan dua Map |

---

## 4. Modul `std/tuple.grd`

Pustaka manipulasi struktur data Tuple (`{A, B, C}`).

| Fungsi | Signature | Deskripsi |
| :--- | :--- | :--- |
| `Tuple.elemen` | `(Tuple, Index) -> Any` | Mengambil elemen pada indeks (0-based) |
| `Tuple.panjang` | `(Tuple) -> Integer` | Mengembalikan jumlah elemen pada Tuple |
| `Tuple.ke_list` | `(Tuple) -> List` | Mengonversi Tuple menjadi List |
| `Tuple.gabung` | `(T1, T2) -> Tuple` | Menggabungkan dua Tuple menjadi Tuple baru |

---

## 5. Modul `std/json.grd`

Pustaka encoder dan decoder JSON format.

| Fungsi | Signature | Deskripsi |
| :--- | :--- | :--- |
| `Json.encode_int` | `(Integer) -> String` | Mengodekan Integer ke String JSON |
| `Json.decode_int` | `(String) -> Integer` | Membaca Integer dari String JSON |
| `Json.encode_string` | `(String) -> String` | Mengodekan String ke JSON String |
| `Json.decode_string` | `(String) -> String` | Membaca String dari JSON String |
| `Json.encode_bool` | `(Boolean) -> String` | Mengodekan Boolean ke JSON |
| `Json.decode_bool` | `(String) -> Boolean` | Membaca Boolean dari JSON |
| `Json.encode_list` | `(List) -> String` | Mengodekan List ke JSON Array |
| `Json.decode_list` | `(String) -> List` | Membaca JSON Array ke List |
| `Json.encode_map` | `(Map) -> String` | Mengodekan Map ke JSON Object |
| `Json.decode_map` | `(String) -> Map` | Membaca JSON Object ke Map |

---

## 6. Modul `std/waktu.grd` & `std/matematika.grd`

### Waktu
- `Waktu.sekarang()` $\rightarrow$ Unix timestamp milidetik.
- `Waktu.tunggu(Ms)` $\rightarrow$ Penundaan thread selama `Ms` milidetik.
- `Waktu.durasi_mulai()` & `Waktu.durasi_selesai(Start)` $\rightarrow$ Pengukuran interval waktu.

### Matematika
- `Matematika.pangkat(Base, Exp)` $\rightarrow$ Perpangkatan.
- `Matematika.abs(X)` $\rightarrow$ Nilai absolut.
- `Matematika.min(A, B)` & `Matematika.max(A, B)` $\rightarrow$ Nilai minimum dan maksimum.
- `Matematika.akar(X)` $\rightarrow$ Akar kuadrat integer.
