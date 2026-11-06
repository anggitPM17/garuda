# 🦅 GARUDA NATIVE HTTP WEB SERVER (100% BAHASA GARUDA)

Dokumen ini membuktikan secara ilmiah dan praktis bahwa **Bahasa Pemrograman Garuda** kini mampu bertindak sebagai **HTTP Web Server Native Murni** yang ditulis 100% dalam skrip Garuda tanpa ketergantungan pada Nginx, Node.js, atau framework C++ eksternal.

---

## 🚀 CARA MENJALANKAN SERVER

```bash
grd run examples/web_server_garuda.grd
```

atau menggunakan biner rilis:

```bash
.\build\Release\grd.exe run examples/web_server_garuda.grd
```

---

## 💻 KODE SUMBER WEB SERVER (100% SKRIP GARUDA)

File [`examples/web_server_garuda.grd`](file:///D:/richat/garuda_v67_0_production/garuda_v67.0.0/examples/web_server_garuda.grd):

```garuda
modul WebServerGaruda

fungsi handle_request(Req) ->
    Path = Map.ambil(Req, "path")
    tinjau Path ing
        "/" ->
            Http.buat_respons(200, %{"Content-Type" => "text/html"}, "<h1>Halo dari Garuda Native HTTP Server!</h1><p>Ditulis 100% dalam bahasa Garuda.</p>")
        "/health" ->
            Http.buat_respons(200, %{"Content-Type" => "application/json"}, "{\"status\":\"ok\",\"engine\":\"garuda_native_http\"}")
        "/info" ->
            Http.buat_respons(200, %{"Content-Type" => "application/json"}, "{\"language\":\"Garuda\",\"version\":\"v67.0.0\",\"server\":\"native\"}")
        _ ->
            Http.buat_respons(404, %{"Content-Type" => "text/plain"}, "404 Not Found - Garuda Native HTTP Server")
    rampung
rampung

fungsi utama() ->
    cetak("🦅 Garuda Native HTTP Server starting on port 9090...")
    cetak("Coba akses: http://localhost:9090/")
    cetak("Health check: http://localhost:9090/health")
    cetak("Info: http://localhost:9090/info")
    Server.mulai(9090, handle_request, 1000)
rampung
```

---

## 📊 BUKTI OUTPUT HASIL PENGUJIAN CURL (5/5 PASS)

### 1. Request Halaman Utama (`GET /`)
```bash
$ curl -s http://localhost:9090/
<h1>Halo dari Garuda Native HTTP Server!</h1><p>Ditulis 100% dalam bahasa Garuda.</p>
```

### 2. Request Health Check Endpoint (`GET /health`)
```bash
$ curl -s http://localhost:9090/health
{"status":"ok","engine":"garuda_native_http"}
```

### 3. Request Informasi Runtime (`GET /info`)
```bash
$ curl -s http://localhost:9090/info
{"language":"Garuda","version":"v67.0.0","server":"native"}
```

### 4. Request Route Tidak Ditemukan (`GET /tidak-ada`)
```bash
$ curl -s http://localhost:9090/tidak-ada
404 Not Found - Garuda Native HTTP Server
```

### 5. Request HTTP Header Check (`HEAD /`)
```bash
$ curl -sI http://localhost:9090/
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 85
```

---

## 🛠️ TABEL BUILT-IN FUNCTIONS (BIF) YANG DIGUNAKAN

| BIF | Deskripsi & Fungsi |
|---|---|
| `Server.mulai(port, handler_fn)` | Membuka listener port TCP dan melakukan dispatching ke fungsi Garuda `handler_fn(Req)` |
| `Http.parse_request(raw_str)` | Mengurai string HTTP request mentah menjadi `Map` (`%{"metode" => ..., "path" => ..., "headers" => ..., "body" => ...}`) |
| `Http.buat_respons(status, headers, body)` | Membentuk string HTTP response standar beserta header `Content-Length` otomatis |
| `Map.ambil(map, kunci)` | Mengambil nilai dari struktur data Map Garuda |
| `Soket.buka`, `terima`, `baca`, `tulis`, `tutup` | Fondasi BIF Low-Level Socket I/O C++ |
