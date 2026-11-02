# Web Framework & HTTP Services Bahasa Garuda

Dokumentasi penggunaan Web Framework (`std/web.grd`) dan HTTP Client/Server pada Bahasa Garuda v67.0.0.

---

## 1. Overview Web Framework

Bahasa Garuda menyediakan modul `std/web.grd` untuk membangun RESTful API dan layanan web berkinerja tinggi. Web Framework Garuda mendukung pendaftaran rute dinamis (*HTTP GET/POST*) serta pendaftaran handler Garuda yang secara thread-safe dieksekusi di atas GVM.

---

## 2. Struktur Pendaftaran Handler & Routing

Pendaftaran rute dilakukan menggunakan `Web.get` dan `Web.post`, yang menghubungkan path URL ke nama handler yang terdaftar via `Web.daftar_handler`:

```garuda
modul WebApp
impor "std/web.grd"

fungsi utama() ->
    -- 1. Daftarkan kode handler Garuda
    Web.daftar_handler("h_sapaan", "\"Halo dari Garuda Web Backend!\"")
    Web.daftar_handler("h_hitung", "40 + 2")

    -- 2. Daftarkan rute GET/POST
    Web.get("/sapa", "h_sapaan")
    Web.get("/hitung", "h_hitung")

    -- 3. Jalankan HTTP Web Server pada port 8080
    Web.jalankan(8080)
rampung
```

---

## 3. Contoh REST API JSON

```garuda
modul RestServer
impor "std/web.grd"
impor "std/json.grd"
impor "std/map.grd"

fungsi utama() ->
    -- Handler mengembalikan JSON
    DataPengguna = Json.encode_list(["Ahmad", "Budi", "Siti"])
    StatusRespon = Json.encode_map(Map.taruh(Map.baru(), "status", "online"))

    Web.daftar_handler("h_pengguna", DataPengguna)
    Web.daftar_handler("h_status", StatusRespon)

    Web.get("/api/pengguna", "h_pengguna")
    Web.get("/api/status", "h_status")

    Web.jalankan(8080)
rampung
```

---

## 4. HTTP Client (`std/http.grd`)

Bahasa Garuda menyediakan HTTP Client bawaan untuk melakukan konsumsi API eksternal:

```garuda
modul KonsumenAPI
impor "std/http.grd"

fungsi utama() ->
    -- HTTP GET
    ResponBody = Http.ambil("http://127.0.0.1:8080/api/status")
    cetak("Hasil GET: " ++ ResponBody)

    -- HTTP POST JSON
    ResponPost = Http.kirim_json("http://127.0.0.1:8080/api/simpan", "{\"nama\":\"Garuda\"}")
    cetak("Hasil POST: " ++ ResponPost)
rampung
```

---

## 5. Thread Safety & Arsitektur Internal

- Server HTTP mendengarkan *incoming connections* menggunakan `cpp-httplib` pada thread latar belakang.
- Saat request diterima, runtime GVM mengeksekusi handler skrip Garuda secara terisolasi dan *thread-safe* dengan pelindung `std::mutex`.
