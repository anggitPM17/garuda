# 🌌 ROADMAP UTUH: JALUR LANGIT
> **Misi:** Membangun Database Engine mandiri untuk Garuda yang setara dengan PostgreSQL, namun dioptimalkan untuk Model Aktor, WebSocket, dan AI Autopilot.
> **Filosofi:** "Interop > Reinvent" (Gunakan In-Memory untuk kecepatan, WAL untuk durabilitas, dan Nexus untuk komunikasi).

---

## 🏛️ ARSITEKTUR TINGKAT 1 - 7 (CORE ENGINE)
Kita membangun database ini secara bertahap dari yang paling sederhana hingga paling kompleks.

- [x] **Tingkat 1: Fondasi Storage (In-Memory)** 
  *   Menggunakan `std::map` berlapis dengan `std::mutex` untuk keamanan *thread*.
  *   Menyimpan data dalam format `Row` (Key-Value).
- [x] **Tingkat 2: Schema Manager**
  *   Mendefinisikan `ColumnDef` (Nama, Tipe, Primary Key).
  *   Mencegah pembuatan tabel ganda.
- [x] **Tingkat 3: Transaction Manager (ACID Dasar)**
  *   Mendukung `BEGIN`, `COMMIT`, dan `ROLLBACK`.
  *   Data ditahan di *uncommitted changes* sebelum masuk ke Storage.
- [x] **Tingkat 4: B-Tree Index**
  *   Menggunakan `std::multimap` untuk pencarian cepat (O(log n)).
- [x] **Tingkat 5: Write-Ahead Log (WAL)**
  *   Menulis setiap operasi ke file `.log` sebelum eksekusi.
  *   Mencegah kehilangan data jika server mati mendadak.
- [x] **Tingkat 6: Query Engine**
  *   *Parser* sederhana yang menerjemahkan string perintah (INSERT, SELECT) menjadi aksi.
- [x] **Tingkat 7: Ultimate Database (`JalurLangitUltimate`)**
  *   Kelas utama yang menyatukan Storage, Schema, Transaction, Index, WAL, dan Query Engine.

---

## 🛠️ 6 FUNGSI PENUNJANG (ENTERPRISE INFRASTRUCTURE)
Database tidak akan berguna tanpa "aksesoris" ini. Semua fitur ini wajib ada di folder `include/garuda/jalur_langit/`.

1.  **[x] Connection Pooler (`connection_pool.hpp`)**
    *   **Fungsi:** Mengatur antrean koneksi agar RICHAT tidak *crash* saat diakses 10.000 pengguna.
    *   **Mekanisme:** Menggunakan `std::condition_variable`. Pinjam koneksi -> Pakai -> Kembalikan.
2.  **[x] Query Cache (`query_cache.hpp`)**
    *   **Fungsi:** Menyimpan hasil query yang sering dipanggil di RAM.
    *   **Mekanisme:** Menggunakan TTL (*Time-To-Live*). Jika cache kadaluarsa, hapus dan ambil dari disk.
3.  **[x] Auth & Permission Manager (`auth_manager.hpp`)**
    *   **Fungsi:** Setara `GRANT` dan `REVOKE` di PostgreSQL. Menjaga agar Aktor A tidak bisa menghapus data Aktor B.
4.  **[x] Backup & Restore (`backup_manager.hpp`)**
    *   **Fungsi:** *Disaster Recovery*. Meng- *copy* file `.wal` dan storage ke folder *backup* secara berkala.
5.  **[x] Schema Migrator (`schema_migrator.hpp`)**
    *   **Fungsi:** Mengelola versi database. Jika RICHAT di- *update* (nambah kolom `is_read`), migrator memastikan data lama tidak rusak.
6.  **[x] JSON Serializer (`json_serializer.hpp`)**
    *   **Fungsi:** Mengubah `Row` (data tabel) menjadi string JSON secara otomatis.
    *   **Kegunaan:** Agar data dari Jalur Langit bisa langsung dikirim ke RICHAT lewat WebSocket tanpa perlu diubah manual.

---

## 🚦 FASE PENGEMBANGAN (LANGKAH KERJA AGEN AI)

Jangan minta agen mengerjakan semuanya sekaligus. Berikan instruksi bertahap (satu fase per sesi) agar agen tidak *error* dan bisa lulus tes satu per satu.

### 📍 FASE 1: Core Engine & WAL (Hari ke-1)
*   **Target:** Mengimplementasikan `jalur_langit_ultimate.hpp`.
*   **Tes:** `test_jalur_langit.cpp` (Membuat tabel, insert data, memastikan file `wal.log` terbentuk).

### 📍 FASE 2: Infrastructure (Hari ke-2) - [x] SELESAI
*   **Target:** Implementasi `connection_pool.hpp`, `query_cache.hpp`, `json_serializer.hpp`.
*   **Tes:** `test_connection_pool.cpp`, `test_query_cache.cpp`, `test_json_serializer.cpp`.
*   **Verifikasi:** Pastikan cache mengembalikan data yang sama dengan database, dan JSON terbentuk dengan benar.

### 📍 FASE 3: Keamanan & Manajemen (Hari ke-3) - [x] SELESAI
*   **Target:** Implementasi `auth_manager.hpp`, `backup_manager.hpp`, `schema_migrator.hpp`.
*   **Tes:** `test_auth_manager.cpp`, `test_backup_manager.cpp`, `test_schema_migrator.cpp`.
*   **Verifikasi:** Pastikan user tanpa izin gagal saat mencoba query, dan file backup berhasil dibuat.

### 📍 FASE 4: Integrasi Garuda Nusantara (Hari ke-4) - [x] SELESAI
*   **Target:** Menghubungkan Jalur Langit ke `NusantaraBroker`. 
*   **Skenario:** Autopilot AI mengirim pesan ke Nusantara untuk meng- *update* skema Jalur Langit secara *live*, lalu RICHAT menerima notifikasi via JSON Serializer.

### 📍 FASE 5.1: Autopilot AI — AdminGate + HealthMonitor - [x] SELESAI
*   **Target:** `include/garuda/autopilot/admin_gate.hpp`, `health_monitor.hpp`.
*   **Tes:** `test_admin_gate.cpp`, `test_health_monitor.cpp`.
*   **Verifikasi:** Mode gate bekerja (MANUAL/AUTOPILOT/STOP), health check fail-safe.

---

## 📋 PROMPT UNTUK AGEN AI (SALIN & TEMPEL BESOK)

> **Konteks:** Hari ini kita akan mulai membangun **Jalur Langit Database Engine** sesuai `ROADMAP_JALUR_LANGIT.md`. Ini adalah pengganti PostgreSQL untuk proyek Garuda.
>
> **Tugas Hari Ini (FASE 1):**
> 1. Buat folder `include/garuda/jalur_langit/`.
> 2. Simpan kode `jalur_langit_ultimate.hpp` yang sudah saya siapkan (berisi Tingkat 1-7).
> 3. Buat file tes `tests/test_jalur_langit.cpp`. Tes harus memverifikasi bahwa:
>    - Tabel bisa dibuat (`create_table`).
>    - Insert query berjalan (`execute`).
>    - File WAL (`wal.log`) benar-benar terbentuk di direktori data.
> 4. Tambahkan target `garuda_jalur_langit_tests` ke `CMakeLists.txt`, `scripts/run_tests.ps1`, dan `.github/workflows/ci.yml`.
> 5. Build dan jalankan tesnya. Pastikan tes **PASS (Hijau)**.
> 6. **JANGAN SENTUH** `script_engine.hpp` atau file di luar folder `jalur_langit`.
> 7. Laporkan output `run_tests.ps1` dan `git status` di akhir sesi.
