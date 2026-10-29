# 📚 RICHAT API Reference Guide

Dokumen ini berisi daftar lengkap endpoint REST API dan protokol WebSocket yang didukung oleh **Garuda BEAM C++ Production Server**.

---

## 🌐 1. Base URLs

- **Production HTTPS**: `https://richat.my.id`
- **Production WSS**: `wss://richat.my.id/`
- **Local Dev HTTP**: `http://localhost:9002`

---

## 🔒 2. REST HTTP Endpoints

### 🟢 `GET /health`
Mengecek status kesehatan runtime server.
- **Request**: `curl -s https://richat.my.id/health`
- **Response `200 OK`**:
  ```json
  {
    "status": "ok",
    "engine": "garuda_beam_v67",
    "app": "richat"
  }
  ```

### 📊 `GET /metrics`
Mengekspor metrik telemetry dalam format Prometheus.
- **Request**: `curl -s https://richat.my.id/metrics`
- **Response `200 OK`**:
  ```text
  # HELP garuda_beam_up Garuda BEAM status
  # TYPE garuda_beam_up gauge
  garuda_beam_up 1
  ```

### 🔑 `POST /api/auth/login`
Otentikasi pengguna.
- **Request**: `curl -X POST https://richat.my.id/api/auth/login -H "Content-Type: application/json" -d '{"phone":"+62812345678","password":"secretpassword"}'`
- **Response `200 OK`**:
  ```json
  {
    "status": "ok",
    "success": true,
    "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
    "user_id": "usr_richat_2026",
    "display_name": "Garuda User"
  }
  ```

### 🔑 `POST /api/auth/register`
Pendaftaran pengguna baru.
- **Request**: `curl -X POST https://richat.my.id/api/auth/register -H "Content-Type: application/json" -d '{"phone":"+62888888888","password":"newpassword"}'`
- **Response `200 OK`**:
  ```json
  {
    "status": "ok",
    "success": true,
    "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
    "user_id": "usr_richat_2026",
    "display_name": "Garuda User"
  }
  ```

---

## 🔌 3. WebSocket Real-time Protocol (`wss://richat.my.id/`)

### Heartbeat Ping / Pong
- **Ping Message**: `{"type":"ping","ts":1793238336001}`
- **Pong Response**: `{"type":"pong"}`
