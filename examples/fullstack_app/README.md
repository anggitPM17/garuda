# 🦅 Garuda Full-Stack Web Application Demo

Aplikasi Web REST API terintegrasi penuh menggunakan ekosistem pustaka bawaan Bahasa Garuda:
- **`Json`**: Serialisasi & Parsing JSON murni.
- **`HttpServer`**: Server Web HTTP/1.1 & Router *Multi-Worker*.
- **`Http`**: Klien HTTP/1.1 untuk *external API calls*.
- **`Db`**: Engine Penyimpanan *Database Persistence*.

---

## ⚡ Endpoints REST API

| Method | Path | Deskripsi | Format Respons |
|--------|------|-----------|----------------|
| `GET` | `/` | Pengecekan kesehatan (*Health check*) | `{"app":"Garuda Fullstack","status":"ok"}` |
| `POST` | `/users` | Pendaftaran pengguna baru | `{"id":"usr_101","status":"created"}` |
| `GET` | `/users/:id` | Ambil data pengguna berdasarkan ID | `{"id":"usr_101","nama":"Budi","umur":42}` |

---

## 🛠️ Cara Menjalankan

```bash
cd examples/fullstack_app
grdmix run
```
