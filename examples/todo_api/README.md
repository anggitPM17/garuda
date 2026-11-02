# Examples: Todo List REST API (`examples/todo_api`)

Contoh aplikasi RESTful Web API Todo List berbasis `std/web.grd`, `std/json.grd`, dan `std/map.grd`.

## Endpoint

- `GET /todos` — Mengembalikan daftar seluruh todo (JSON Array)
- `GET /todo/1` — Mengembalikan detail todo ID 1 (JSON Object)
- `GET /stats` — Mengembalikan statistik layanan (JSON Object)

## Cara Menjalankan

```powershell
.\build\Release\grd.exe run examples/todo_api/utama.grd
```

## Pengujian HTTP Request

```powershell
Invoke-WebRequest -Uri "http://127.0.0.1:8080/todos" -UseBasicParsing
```
