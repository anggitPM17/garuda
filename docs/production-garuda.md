# 🚀 GARUDA PRODUCTION-READY WEB SERVER GUIDE (GVM ENGINE)

Dokumen ini menjelaskan penggunaan fitur **Production-Readiness Web Infrastructure** pada GVM Engine: WebSocket Server (RFC 6455), HTTPS TLS Server, Router & Middleware Pipeline, dan Static File Serving dengan Proteksi Path Traversal.

---

## 1. 🔌 WEBSOCKET SERVER (RFC 6455)

```garuda
modul DemoWS

fungsi handle_ws(Conn, Msg) ->
    tinjau Msg ing
        "ping" -> Ws.kirim(Conn, "pong")
        _ -> Ws.kirim(Conn, "Echo: " ++ Msg)
    rampung
rampung

fungsi utama() ->
    Server.mulai_ws(9092, handle_ws)
rampung
```

### Spesifikasi Teknis WebSocket (Fase 10.8b):
- **Fragmentation Support (`FIN=0`)**: Otomatis menggabungkan fragmen payload text (`0x1`) & binary (`0x2`) hingga fragmen `FIN=1` (`0x0` continuation) diterima dari browser (Chrome, Firefox, Safari).
- **Max Payload Bound Security (Anti-DoS)**: Dibatasi maksimal 16 MB (`16,777,216` byte). Request melebihi batas langsung ditolak dengan Close Frame RFC 6455 `1009 (MESSAGE_TOO_BIG)` tanpa alokasi memori berlebih.
- **Socket Timeout**: Socket client diberi timeout `SO_RCVTIMEO` 5 detik untuk mencegah koneksi hang.

---

## 2. 🔒 HTTPS TLS SERVER (`Server.mulai_https`)

```garuda
modul DemoHTTPS

fungsi handle_request(Req) ->
    Http.buat_respons(200, %{"Content-Type" => "text/html"}, "<h1>Koneksi Aman HTTPS Garuda</h1>")
rampung

fungsi utama() ->
    Server.mulai_https(443, "/path/to/cert.pem", "/path/to/key.pem", handle_request)
rampung
```

### Spesifikasi Teknis HTTPS (Fase 10.8b):
- **TLS-Only Strict**: Tidak ada fallback HTTP Plaintext. Koneksi non-TLS atau handshake gagal langsung ditutup rapat.
- **Handshake Error Logging**: Error TLS handshake dicetak bersama `mbedtls_strerror` ke file log jika variabel lingkungan `GARUDA_HTTPS_ERROR_LOG` diaktifkan.

---

## 3. 🛤️ ROUTER & MIDDLEWARE PIPELINE (`Router`)

```garuda
modul DemoRouter

fungsi utama() ->
    R = Router.baru()
        |> Router.get("/", "handler_home")
        |> Router.get("/users/:id", "handler_user")
        |> Router.get("/files/:category/:name", "handler_file")
        |> Router.post("/users", "handler_create")
        |> Router.middleware("logging")
        |> Router.middleware("cors")
    
    Req = %{"path" => "/files/images/logo.png"}
    Res = Router.dispatch(R, Req)
    cetak("Respons: #{Res}")
rampung
```

---

## 4. 📁 STATIC FILE SERVING WITH SECURITY (`Server.mulai_async_statis`)

```garuda
modul DemoStatic

fungsi utama() ->
    // Melayani file statis dari folder site/ dengan proteksi otomatis terhadap path traversal (../)
    Server.mulai_async_statis(9093, "site/")
rampung
```

---

## ⚙️ CATATAN & BATASAN PRODUKSI (FASE 10.8b STABLE)

| Komponen | Status | Catatan Produksi |
|---|---|---|
| **WebSocket Fragmentation** | ✅ **Verified** | Mendukung `FIN=0` multi-fragment teruji via Python `websockets` client |
| **WebSocket Max Bound** | ✅ **16 MB Limit** | Close Frame `1009 MESSAGE_TOO_BIG` dikirim jika payload >16MB |
| **HTTPS TLS Strict** | ✅ **TLS Murni** | Non-TLS ditolak total; error handshake di-log via `GARUDA_HTTPS_ERROR_LOG` |
| **Socket Receive Timeout** | ✅ **5s Timeout** | Socket client dikonfigurasi `SO_RCVTIMEO` 5s untuk mencegah hanging sockets |
| **Default `max_reqs`** | ℹ️ **100 Reqs** | Untuk server production long-running, set `max_reqs` lebih tinggi (misal `1000000`) |
