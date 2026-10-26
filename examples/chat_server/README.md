# 🦅 Chat Server Garuda — Production Case Study

Server Obrolan Terdistribusi Production-Grade berbasis *Concurrent Actor Model* Garuda.

---

## ⚡ Arsitektur Sistem

```text
┌─────────────────┐
│  Chat Server    │ <── Broadcast Pesan
└────────┬────────┘
         │
    ┌────┴────┐
    │         │
┌───▼───┐ ┌───▼───┐
│ Budi  │ │  Ani  │
└───────┘ └───────┘
```

---

## 🚀 Fitur Utama

- **Multi-Actor Concurrency**: Server pusat & tiap pengguna berjalan sebagai proses aktor terisolasi.
- **Supervisor Fault Tolerance**: Pengawasan aktor dengan pemulihan/restart otomatis.
- **Broadcasting**: Penyebaran pesan *real-time* ke seluruh pengguna aktif.

---

## 🛠️ Cara Menjalankan

```bash
cd examples/chat_server
grdmix run
```
