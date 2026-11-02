# Sintaks & Struktur Bahasa Garuda

Dokumentasi resmi sintaksis dan struktur Bahasa Garuda v67.0.0.

---

## 1. Modul & Deklarasi

Setiap berkas skrip Bahasa Garuda (`.grd`) merupakan sebuah modul yang diawali dengan deklarasi nama modul:

```garuda
modul NamaModul

fungsi utama() ->
    "Halo Dunia"
rampung
```

## 2. Fungsi

Fungsi dideklarasikan menggunakan kata kunci `fungsi`, diikuti oleh nama fungsi, parameter, operator tanda panah `->`, badan fungsi, dan ditutup dengan `rampung`:

```garuda
fungsi tambah(A, B) ->
    A + B
rampung
```

Nilai balik fungsi secara otomatis merupakan hasil evaluasi ekspresi terakhir di dalam badan fungsi.

## 3. Variabel & Tipe Data

Bahasa Garuda memiliki tipe data dinamis dan imutabel (*immutable-by-default*):

```garuda
Umur = 25                                -- Integer
RPS = 191.65                             -- Double/Float
Nama = "Bahasa Garuda"                   -- String
Aktif = benar                            -- Boolean (benar / salah)
DataKosong = nil                         -- Nil
Status = :ok                             -- Atom (:ok, :error)
DaftarAngka = [1, 2, 3, 4]              -- List
Pasangan = {"Garuda", 2026}              -- Tuple
MapData = Map.taruh(Map.baru(), "k1", 1) -- Map
```

## 4. Percabangan (`yen` & `lain`)

Percabangan `if-else` dinyatakan dengan kata kunci `yen` dan `lain`:

```garuda
fungsi cek_nilai(N) ->
    yen N >= 70 ->
        "Lulus"
    lain ->
        "Ulang"
    rampung
rampung
```

## 5. Guard Clause (`nalika` / `ketika`)

*Guard clause* memeriksa prasyarat parameter sebelum badan fungsi dieksekusi. Jika guard bernilai `salah`, fungsi mengembalikan `nil`:

```garuda
fungsi bagi(A, B) nalika B != 0 ->
    A / B
rampung

fungsi akar(X) ketika X >= 0 ->
    Matematika.akar(X)
rampung
```

## 6. Pipe Operator (`|>`)

*Pipe operator* mengalirkan hasil ekspresi sebelah kiri sebagai argumen pertama ke fungsi di sebelah kanan:

```garuda
fungsi dbl(X) -> X * 2 rampung

fungsi utama() ->
    (10 + 20) |> dbl                     -- Mengembalikan 60
    [1, 2, 3] |> List.balik |> List.panjang -- Mengembalikan 3
rampung
```

## 7. Lambda & Higher-Order Functions

Lambda anonim dibuat menggunakan `fungsi(...) -> ... rampung`:

```garuda
impor "std/list.grd"

fungsi utama() ->
    Daftar = [1, 2, 3, 4, 5]
    KaliDua = List.peta(Daftar, fungsi(X) -> X * 2 rampung)
    HanyaGenap = List.saring(Daftar, fungsi(X) -> X % 2 == 0 rampung)
rampung
```

## 8. Actor Model Concurrency

Setiap proses di GVM dijalankan sebagai Actor independen berlatensi rendah:

```garuda
fungsi utama() ->
    -- Spawn proses actor baru
    Pid = cakra fungsi() ->
        tampa
            {ping, Sender} -> kirim(Sender, :pong)
            _ -> :abaikan
        rampung
    rampung

    -- Kirim pesan ke PID
    kirim(Pid, {ping, diri()})
rampung
```

## 9. String Interpolation

String mendukung interpolasi variabel atau ekspresi secara langsung menggunakan `#{...}`:

```garuda
Skor = 100
Nama = "Budi"
Pesan = "Pemain #{Nama} meraih skor #{Skor}!"  -- "Pemain Budi meraih skor 100!"
```

## 10. Tabel Kata Kunci Resmi

| Kata Kunci | Fungsi Sintaksis |
| :--- | :--- |
| `modul` | Deklarasi nama modul |
| `fungsi` | Deklarasi fungsi / lambda |
| `makro` | Deklarasi makro metaprogramming |
| `rampung` | Penutup blok deklarasi / percabangan |
| `yen` / `lain` | Kata kunci percabangan (if / else) |
| `nalika` / `ketika` | Guard clause pada argumen fungsi (when) |
| `tinjau` / `ing` | Pattern matching (case / in) |
| `cakra` | Spawn proses Actor baru di GVM |
| `kirim` | Mengirim pesan ke PID Actor |
| `tampa` | Menerima pesan pada mailbox Actor |
| `setelah` | Timeout pada penerimaan pesan Actor |
| `diri` | Mengembalikan PID Actor saat ini |
| `impor` | Mengimpor modul pustaka standar |
| `benar` / `salah` | Literal Boolean `true` / `false` |
| `nil` | Literal `null` / `undefined` |
