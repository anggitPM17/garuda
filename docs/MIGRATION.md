# 🗄️ RICHAT Data Migration Guide: PostgreSQL (Erlang) -> Mnesia (Garuda)

Dokumen ini menjelaskan prosedur migrasi data pengguna, pesan, dan grup dari PostgreSQL Erlang/OTP ke Mnesia Garuda BEAM.

---

## 🛠️ Prosedur Migrasi

1. **Jalankan Script Migrasi**:
   ```bash
   chmod +x scripts/migrate_postgres_to_mnesia.sh
   ./scripts/migrate_postgres_to_mnesia.sh
   ```

2. **File Output Export**:
   - `/tmp/users_export.json` (Daftar pengguna terdaftar)
   - `/tmp/messages_export.json` (Riwayat pesan obrolan)

3. **Integritas Mnesia**:
   - Garuda Mnesia menggunakan tabel `richat_user`, `richat_contact`, `richat_moment_post`, `richat_status_story`, `richat_group`, `richat_group_member`, `richat_auto_reply`, dan `offline_message`.
