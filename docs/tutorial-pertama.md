# Tutorial Pertama — Membangun Aplikasi Todo List CLI dalam 30 Menit

Tutorial *step-by-step* ini akan memandu Anda membangun aplikasi Todo List CLI interaktif menggunakan Bahasa Garuda v67.0.0.

---

## 🎯 Fitur Aplikasi yang Akan Dibangun

- Penambahan tugas baru ke dalam memori Map.
- Pengambilan item tugas berdasarkan ID.
- Penghapusan tugas dari memori.
- Pemrosesan instruksi menggunakan *Pattern Matching* (`tinjau`/`ing`).

---

## Langkah 1: Persiapan Proyek (2 Menit)

Buat direktori baru untuk proyek aplikasi todo Anda:

```powershell
mkdir todo_app
cd todo_app
```

Buat berkas `utama.grd` dengan isi dasar:

```garuda
modul TodoApp

fungsi utama() ->
    "Aplikasi Todo List Bahasa Garuda v67.0.0"
rampung
```

Jalankan skrip:
```powershell
..\build\Release\grd.exe run utama.grd
```
Output:
```text
=> "Aplikasi Todo List Bahasa Garuda v67.0.0"
```

---

## Langkah 2: Menggunakan `std/map.grd` untuk Storage (10 Menit)

Ubah `utama.grd` untuk mengimpor modul `std/map.grd` dan mengelola penyimpanan data tugas key-value:

```garuda
modul TodoApp
impor "std/map.grd"

fungsi tambah_tugas(Storage, Id, Judul) ->
    Map.taruh(Storage, Id, Judul)
rampung

fungsi ambil_tugas(Storage, Id) ->
    Map.ambil(Storage, Id)
rampung

fungsi hapus_tugas(Storage, Id) ->
    Map.hapus(Storage, Id)
rampung

fungsi utama() ->
    -- Inisialisasi Map kosong
    Tugas0 = Map.baru()

    -- Tambahkan dua tugas baru
    Tugas1 = tambah_tugas(Tugas0, "1", "Belajar Bahasa Garuda")
    Tugas2 = tambah_tugas(Tugas1, "2", "Bangun REST Web Backend")

    -- Ambil tugas ID "2"
    ambil_tugas(Tugas2, "2")
rampung
```

Jalankan skrip:
```powershell
..\build\Release\grd.exe run utama.grd
```
Output:
```text
=> "Bangun REST Web Backend"
```

---

## Langkah 3: Menggunakan Pattern Matching (`tinjau`/`ing`) (10 Menit)

Gunakan *Pattern Matching* untuk memproses berbagai perintah (*command pattern*) secara deklaratif:

```garuda
modul TodoApp
impor "std/map.grd"

fungsi proses_instruksi(Instruksi, Storage) ->
    tinjau Instruksi ing
        {tambah, Id, Judul} -> Map.taruh(Storage, Id, Judul)
        {hapus, Id} -> Map.hapus(Storage, Id)
        {ambil, Id} -> Map.ambil(Storage, Id)
        _ -> Storage
    rampung
rampung

fungsi utama() ->
    S0 = Map.baru()
    S1 = proses_instruksi({tambah, "1", "Belajar Bahasa Garuda"}, S0)
    S2 = proses_instruksi({tambah, "2", "Bangun REST Web Backend"}, S1)
    
    -- Ambil tugas "1"
    proses_instruksi({ambil, "1"}, S2)
rampung
```

Jalankan skrip:
```powershell
..\build\Release\grd.exe run utama.grd
```
Output:
```text
=> "Belajar Bahasa Garuda"
```

---

## Langkah 4: Rangkuman & Langkah Selanjutnya (8 Menit)

🎉 **Selamat!** Anda telah berhasil menguasai:
- Deklarasi modul dan fungsi pada Bahasa Garuda.
- Penggunaan modul pustaka standar `std/map.grd`.
- Struktur data imutabel Map (`Map.baru`, `Map.taruh`, `Map.ambil`, `Map.hapus`).
- *Pattern Matching* menggunakan `tinjau`/`ing` dan Tuple `{tambah, Id, Judul}`.

### Langkah Selanjutnya
- Ubah aplikasi ini menjadi Web API menggunakan **[Web Framework (docs/web-framework.md)](web-framework.md)**.
- Lihat contoh versi REST API lengkap pada **[examples/todo_api/](../examples/todo_api/)**.
