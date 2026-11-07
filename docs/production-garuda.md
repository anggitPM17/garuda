# 🚀 GARUDA PRODUCTION-READY WEB SERVER GUIDE (GVM ENGINE)

Dokumen ini menjelaskan penggunaan fitur **Production-Readiness Web Infrastructure** pada GVM Engine: WebSocket Server (RFC 6455 & RFC 7692 Deflate), HTTPS TLS Server, HTTP/2 Multiplexing Server, Async Event Loop Engine, Router & Middleware Pipeline, Static File Serving, dan ETS In-Memory Table.

---

## 1. 🔌 WEBSOCKET SERVER (RFC 6455 & RFC 7692 `permessage-deflate`)

```garuda
modul DemoWS

fungsi handle_ws(Conn, Msg) ->
    tinjau Msg ing
        "ping" -> Ws.kirim(Conn, "pong")
        _ -> Ws.kirim(Conn, "Echo: " ++ Msg)
    rampung
rampung

fungsi utama() ->
    // WebSocket Server dengan kompresi permessage-deflate
    Server.mulai_ws_deflate(9097, handle_ws)
rampung
```

### Spesifikasi Teknis WebSocket (Fase 11):
- **Fragmentation Support (`FIN=0`)**: Otomatis menggabungkan fragmen payload text (`0x1`) & binary (`0x2`) hingga fragmen `FIN=1` (`0x0` continuation) diterima dari browser.
- **Max Payload Bound Security (Anti-DoS)**: Dibatasi maksimal 16 MB (`16,777,216` byte). Request melebihi batas langsung ditolak dengan Close Frame RFC 6455 `1009 (MESSAGE_TOO_BIG)`.
- **`permessage-deflate` (RFC 7692)**: Kompresi payload otomatis mengurangi konsumsi *bandwidth* hingga >50% untuk payload JSON/Teks besar.

---

## 2. ⚡ ASYNC EVENT-LOOP ENGINE (`Server.mulai_async_event_loop`)

```garuda
modul DemoEventLoop

fungsi utama() ->
    // Server Async berbasis I/O multiplexing WSAPoll/poll (menampung 1000+ koneksi simultan)
    Server.mulai_async_event_loop(9095, nil, 1000)
rampung
```

---

## 3. 🌐 HTTP/2 MULTIPLEXING SERVER (`Server.mulai_http2`)

```garuda
modul DemoHttp2

fungsi utama() ->
    // Server HTTP/2 dengan dukungan stream multiplexing & fallback HTTP/1.1 otomatis
    Server.mulai_http2(9096, "cert.pem", "key.pem", nil)
rampung
```

---

## 4. 🗄️ IN-MEMORY TABLE (`Ets` - Erlang Term Storage)

```garuda
modul DemoETS

fungsi utama() ->
    T = Ets.baru()
    Ets.sisip(T, "user_101", "Anggit")
    Ets.sisip(T, "role_101", "Developer")

    Nama = Ets.cari(T, "user_101")
    cetak("User: #{Nama}")

    Ets.hapus(T, "role_101")
    Semua = Ets.semua(T)
rampung
```

---

## ⚙️ CATATAN & BATASAN PRODUKSI (FASE 11 STABLE)

| Komponen | Status | Catatan Produksi |
|---|---|---|
| **Async Event-Loop** | ✅ **1000+ Conns** | Multi-threaded WSAPoll/poll I/O multiplexing dengan memori delta <3MB |
| **HTTP/2 Multiplexing** | ✅ **H2 Verified** | Handshake `PRI * HTTP/2.0`, SETTINGS exchange, stream 1..N multiplexing & fallback HTTP/1.1 |
| **WebSocket Deflate** | ✅ **RFC 7692** | Kompresi payload `permessage-deflate` dengan flag `RSV1=1` & fallback uncompressed |
| **ETS In-Memory Table** | ✅ **Thread-Safe** | Didukung `std::shared_mutex` (`shared_lock` read / `unique_lock` write) bebas race condition |
