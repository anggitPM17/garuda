# 🦅 Infrastruktur Terdistribusi Garuda (`net/`)

Dokumen ini menjelaskan komponen *Distributed System* pada Garuda Engine untuk *deployment* kluster multi-node VPS.

---

## 1. Komponen Utama Kluster

1. **Raft Consensus Protocol (`raft.hpp`)**:
   - Pemilihan Pemimpin (*Leader Election*) dengan batas waktu acak 150-300 ms.
   - Replikasi log konsensus terdistribusi ke seluruh *follower nodes*.
   - Toleransi partisi jaringan (Kluster 2/3 mayoritas tetap dapat berkomit log).
   - Penyelamatan state ke disk (`save_state` & `load_state`).

2. **Global Process Registry (`global_registry.hpp`)**:
   - Pendaftaran nama proses global (`daftar("nama", pid)`).
   - Pencarian PID lintas node (`cari("nama")`).
   - Sinkronisasi otomatis berbasis *Raft Apply Callback*.

3. **TCP Socket Transport Layer (`socket_layer.hpp`)**:
   - Transport TCP asli lintas platform (*Winsock2* di Windows, *BSD Sockets* di Linux).
   - Autentikasi token *handshake* saat node terhubung.
   - Protokol pengiriman berbasis baris (`\n`).

4. **Distributed PubSub Engine (`distributed_pubsub.hpp`)**:
   - Penyebaran pesan *real-time* ke ribuan aktor pelanggan (`subscribe`, `unsubscribe`, `publish`).
   - *Broadcast* pesan lintas server VPS via soket TCP.

5. **Hot Code Reloading (`hot_reload.hpp`)**:
   - Pembaruan versi modul biner `.gvm` di memori secara *live* tanpa menghentikan *Virtual Machine*.
