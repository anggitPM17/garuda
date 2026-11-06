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

---

## 3. 🛤️ ROUTER & MIDDLEWARE PIPELINE (`Router`)

```garuda
modul DemoRouter

fungsi utama() ->
    R = Router.baru()
        |> Router.get("/", "handler_home")
        |> Router.get("/users/:id", "handler_user")
        |> Router.post("/users", "handler_create")
        |> Router.middleware("logging")
        |> Router.middleware("cors")
    
    Req = %{"path" => "/users/42"}
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
