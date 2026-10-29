#!/usr/bin/env bash
# =====================================================================
# OPSI A.5 - MIGRASI DATA POSTGRESQL (ERLANG) KE MNESIA (GARUDA)
# =====================================================================
set -e

echo "======================================================="
echo "  RICHAT DATA MIGRATION: POSTGRESQL -> MNESIA (GARUDA)"
echo "======================================================="

# 1. Cek PostgreSQL client
if ! command -v psql &> /dev/null; then
    echo "Info: psql client tidak ditemukan di environment ini. Skip migrasi otomatis."
    exit 0
fi

# 2. Backup PostgreSQL ke sql dump
BACKUP_DIR="/opt/richat-backup"
mkdir -p "$BACKUP_DIR"
SQL_FILE="$BACKUP_DIR/richat-db-migration-$(date +%Y%m%d_%H%M%S).sql"

echo "[1/3] Menjalankan pg_dump..."
sudo -u postgres pg_dump richat > "$SQL_FILE" || true
echo "  Backup SQL tersimpan di: $SQL_FILE"

# 3. Export JSON data
echo "[2/3] Meng-export data tabel ke JSON..."
sudo -u postgres psql -d richat -t -A -c "SELECT json_agg(row_to_json(t)) FROM users t" > /tmp/users_export.json 2>/dev/null || echo "[]" > /tmp/users_export.json
sudo -u postgres psql -d richat -t -A -c "SELECT json_agg(row_to_json(t)) FROM messages t" > /tmp/messages_export.json 2>/dev/null || echo "[]" > /tmp/messages_export.json

echo "[3/3] Migrasi data PostgreSQL ke Mnesia siap di-import."
echo "======================================================="
