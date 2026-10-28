# 📋 Spesifikasi Kontrak API RICHAT (Single Source of Truth)

**Versi:** 1.0.0  
**Target Service:** Garuda C++ Gateway (R1) & RICHAT Android Kotlin App / Erlang Backend  
**Database Engine:** Jalur Langit Engine v7.0  

---

## 1. Daftar Aksi (Action Enum)

| Action Enum | Deskripsi | Authentication Required |
|---|---|---|
| `RICHAT_LOGIN` | Login user dengan `auth_token` dan `device_id` | No |
| `RICHAT_LOGOUT` | Invalidasi sesi user | Yes |
| `RICHAT_SEND_MESSAGE` | Mengirim pesan instan 1-on-1 | Yes |
| `RICHAT_GET_MESSAGES` | Mengambil riwayat pesan antara dua user | Yes |
| `RICHAT_GET_PRESENCE` | Cek status online/terakhir dilihat user | Yes |
| `RICHAT_ACK_DELIVERED` | Mengonfirmasi pesan telah diterima oleh client | Yes |

---

## 2. Struktur Payload per Aksi

### 2.1 `RICHAT_LOGIN`
- **Request Payload**:
  ```json
  {
    "auth_token": "string (wajib)",
    "device_id": "string (wajib)"
  }
  ```
- **Response Sukses (HTTP 200)**:
  ```json
  {
    "status": "200",
    "success": true,
    "message": "Login successful",
    "data": {
      "user_id": "usr_101",
      "username": "budi",
      "session_token": "sess_892347"
    }
  }
  ```
- **Response Gagal (HTTP 401)**:
  ```json
  {
    "status": "401",
    "success": false,
    "message": "Unauthorized: Invalid auth_token"
  }
  ```

---

### 2.2 `RICHAT_SEND_MESSAGE`
- **Request Payload**:
  ```json
  {
    "session_token": "sess_892347 (wajib)",
    "receiver_id": "usr_102 (wajib)",
    "content": "Halo, pesan ini terenkripsi (wajib, max 4096 karakter)",
    "client_msg_id": "cmsg_001 (wajib, untuk deduplikasi)"
  }
  ```
- **Response Sukses (HTTP 200)**:
  ```json
  {
    "status": "200",
    "success": true,
    "message": "Message sent",
    "data": {
      "message_id": "msg_9001",
      "timestamp": 1793161000,
      "status": "sent"
    }
  }
  ```
- **Response Gagal**:
  - HTTP 400: Field wajib hilang / konten kosong.
  - HTTP 409: Conflict (duplikasi `client_msg_id`).
  - HTTP 413: Content > 4096 karakter.

---

### 2.3 `RICHAT_GET_MESSAGES`
- **Request Payload**:
  ```json
  {
    "session_token": "sess_892347 (wajib)",
    "with_user_id": "usr_102 (wajib)",
    "limit": 50,
    "before_timestamp": 1793161000
  }
  ```
- **Response Sukses (HTTP 200)**:
  ```json
  {
    "status": "200",
    "success": true,
    "data": {
      "messages": [
        {
          "message_id": "msg_9001",
          "sender_id": "usr_101",
          "receiver_id": "usr_102",
          "content": "Halo",
          "timestamp": 1793160000,
          "status": "delivered"
        }
      ]
    }
  }
  ```

---

### 2.4 `RICHAT_ACK_DELIVERED`
- **Request Payload**:
  ```json
  {
    "session_token": "sess_892347 (wajib)",
    "message_id": "msg_9001 (wajib)"
  }
  ```
- **Response Sukses (HTTP 200)**:
  ```json
  {
    "status": "200",
    "success": true,
    "message": "Acknowledged",
    "data": {
      "status": "acknowledged"
    }
  }
  ```

---

## 3. Skema Tabel Jalur Langit (DDL)

1. **Tabel `users`**:
   - `user_id` (STRING, Primary Key)
   - `username` (STRING)
   - `auth_token` (STRING)
   - `device_id` (STRING)
   - `last_seen` (INT64)

2. **Tabel `messages`**:
   - `message_id` (STRING, Primary Key)
   - `sender_id` (STRING)
   - `receiver_id` (STRING)
   - `content` (STRING)
   - `client_msg_id` (STRING)
   - `timestamp` (INT64)
   - `status` (STRING: sent / delivered / read)

3. **Tabel `sessions`**:
   - `session_token` (STRING, Primary Key)
   - `user_id` (STRING)
   - `created_at` (INT64)
   - `expires_at` (INT64)

---

## 4. Aturan Keamanan & Batasan Operasional

1. **Authentication**: Semua request (selain `RICHAT_LOGIN`) wajib menyertakan `session_token` yang sah.
2. **Payload Size Limit**: `content` maksimal **4096 karakter**. Total payload maksimal **1 MB**.
3. **Rate Limiting**: Maksimal **100 pesan / menit** per user.
4. **Deduplikasi**: Berdasarkan `client_msg_id`. Jika `client_msg_id` dari sender yang sama sudah ada, gateway mengembalikan `message_id` yang telah terbuat sebelumnya tanpa membuat duplikat.

---

## 5. Kode Status Error Standard

- **400 Bad Request**: Field wajib hilang / format tidak valid / konten kosong.
- **401 Unauthorized**: Session token invalid / expired / missing.
- **403 Forbidden**: Pengguna tidak memiliki akses ke sumber daya.
- **404 Not Found**: User atau pesan yang diminta tidak ditemukan.
- **409 Conflict**: Duplikasi `client_msg_id`.
- **413 Payload Too Large**: Ukuran `content` > 4096 karakter atau total payload > 1 MB.
- **429 Too Many Requests**: Pengiriman melebihi batas rate limit.
- **500 Internal Server Error**: Kegagalan internal runtime/database.
