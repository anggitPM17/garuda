# 🦅 Referensi Sintaks Bahasa Garuda (`.grd`)

Bahasa Pemrograman Garuda adalah bahasa fungsional berorientasi aktor (*Actor-Model Concurrent Language*) dengan sintaksis dwibahasa Nusantara & Elixir.

---

## 1. Kata Kunci (*Keywords*)

| Kata Kunci Nusantara | Kata Kunci Elixir | Fungsi / Deskripsi |
|----------------------|-------------------|--------------------|
| `modul` | `defmodule` | Deklarasi nama modul |
| `fungsi` | `def` | Deklarasi fungsi atau fungsi anonim |
| `rampung` | `end` | Penutup blok modul, fungsi, `yen`, `tinjau`, atau `tampa` |
| `yen` | `if` | Percabangan kondisional `if` |
| `lain` | `else` | Jalur alternatif `else` |
| `tinjau` | `case` | Percabangan pencocokan pola (*case*) |
| `ing` | `in` | Penunjuk ekspresi subjek `case` |
| `cakra` | `spawn` | Membuat proses aktor baru secara asinkron |
| `tampa` | `receive` | Menerima pesan dari *mailbox* aktor |
| `setelah` | `after` | Batas waktu tunggu (*timeout*) pada penerimaan pesan |
| `diri` | `self` | Mengambil PID proses aktor saat ini |
| `cetak` | `puts` | Merekam dan mencetak output ke konsol |
| `bukan` | `not` | Operator logika negated NOT |
| `atau` | `or` | Operator logika OR |
| `dan` | `and` | Operator logika AND |

---

## 2. Tipe Data (*Data Types*)

```garuda
# 1. Integer (Desimal, Hexadecimal, Binary)
Angka = 42
Hex = 0x1F
Biner = 0b1010

# 2. Float
Pi = 3.14159

# 3. String (UTF-8 dengan Escape Sequence)
Pesan = "Sugeng rawuh, dunia!\n"

# 4. Atom
StatusOk = :ok
StatusErr = :error

# 5. Boolean & Nil
Aktif = benar
Mati = salah
Kosong = nil

# 6. Tuple
DataUser = {1, "Gajah Mada", :aktif}

# 7. List
DaftarAngka = [1, 2, 3, 4, 5]
```

---

## 3. Operator

### Aritmatika
- `+` : Penjumlahan
- `-` : Pengurangan
- `*` : Perkalian
- `/` : Pembagian

### Komparasi
- `==` : Sama dengan
- `!=` : Tidak sama dengan
- `<`  : Kurang dari
- `>`  : Lebih dari
- `<=` : Kurang dari atau sama dengan
- `>=` : Lebih dari atau sama dengan

### Operator Konkurensi & String
- `<-` : Pengiriman pesan (*Send message to PID*)
- `++` : Penggabungan string / list (*Concatenation*)

---

## 4. Struktur Modul & Fungsi

```garuda
modul Matematika

# Fungsi dengan parameter
fungsi tambah(A, B) ->
    A + B
rampung

# Fungsi rekursif dengan TCO (Tail Call Optimization)
fungsi hitung_mundur(N) ->
    yen N > 0 ->
        hitung_mundur(N - 1)
    lain ->
        :selesai
    rampung
rampung
