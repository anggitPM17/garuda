# Examples: Chat Server dengan Actor Concurrency (`examples/chat_server`)

Contoh aplikasi Chat Server yang menggabungkan Actor Model Concurrency GVM (`cakra`, `kirim`, `tampa`) dan REST API `std/web.grd`.

## Cara Menjalankan

```powershell
.\build\Release\grd.exe run examples/chat_server/utama.grd
```

## Pengujian HTTP Request

```powershell
Invoke-WebRequest -Uri "http://127.0.0.1:8081/chat/room" -UseBasicParsing
```
