# 💬 RICHAT Chat & Group API Reference (OPSI A)

Dokumen ini berisi spesifikasi lengkap REST API untuk perpesanan 1-on-1, Group Chat, Real-time Status, dan Offline Queue.

---

## 📩 1. Chat API 1-on-1

- **`POST /api/chat/send`**: Kirim pesan 1-on-1.
  - Body: `{"recipient_id":"usr_b","content":"Halo B"}`
  - Response `200 OK`: `{"id":"msg_101","sender_id":"usr_a","recipient_id":"usr_b","content":"Halo B","status":"sent"}`
- **`GET /api/messages?with=usr_b`**: Ambil riwayat percakapan.
- **`DELETE /api/messages/:id`**: Soft delete pesan.

---

## 👥 2. Group Chat API

- **`POST /api/groups/create`**: Buat grup baru.
  - Body: `{"name":"Grup Garuda","member_ids":["usr_b","usr_c"]}`
- **`POST /api/groups/:id/join`**: Bergabung ke grup.
- **`POST /api/groups/:id/leave`**: Keluar dari grup.
- **`POST /api/groups/:id/messages`**: Kirim pesan grup (fanout).
- **`GET /api/groups/:id/messages`**: Ambil riwayat pesan grup.

---

## ⚡ 3. Real-time Status API

- **`POST /api/presence/online`**: Set status online.
- **`POST /api/presence/offline`**: Set status offline.
- **`POST /api/typing/start`**: Kirim indikator sedang mengetik.
- **`POST /api/typing/stop`**: Hentikan indikator sedang mengetik.
- **`POST /api/receipt/read`**: Tanda centang terbaca (read receipt).
- **`POST /api/reaction/add`**: Tambahkan reaksi emoji pada pesan.
