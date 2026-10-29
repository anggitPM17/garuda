# 🛡️ Garuda BEAM — Advanced Security Architecture (FASE C.5)

Dokumen ini menjelaskan kebijakan dan arsitektur lapisan keamanan tingkat lanjut pada Garuda BEAM dan RICHAT Production Server.

---

## 🔒 1. Rate Limiting (`rate_limiter.hpp`)

- **Kebijakan Default**:
  - API Login / Register: `10 request / 60 detik` per IP address.
  - REST & WebSocket API umum: `100 request / 60 detik` per IP / user session.
- **Tindakan**: Request yang melampaui threshold akan menerima respons HTTP `429 Too Many Requests`.

---

## 🔑 2. Two-Factor Authentication / 2FA TOTP (`totp_2fa.hpp`)

- **Spesifikasi**: Kompatibel dengan RFC 6238 (TOTP Time-Based One-Time Password) & Google Authenticator / Authy.
- **URI Format**: `otpauth://totp/RICHAT:user@domain?secret=BASE32&issuer=RICHAT`
- **Clock Skew Tolerance**: Window ±30 detik.
- **Backup Codes**: 10 kode pemulihan darurat unik 8 karakter.

---

## 🤖 3. CAPTCHA Proof-of-Work (`captcha_pow.hpp`)

- **Arsitektur Hashcash**: Proteksi pendaftaran bot tanpa dependensi layanan pihak ketiga (seperti Google reCAPTCHA) untuk menjamin privasi pengguna.
- **Verifikasi Server**:
  1. Server menerbitkan challenge `id` dan `prefix`.
  2. Klien melakukan iterasi pencarian `nonce` hingga hash SHA-256 berawalan '000' (3-4 leading zeroes).
  3. Server memverifikasi dalam O(1) time dan langsung menghapus challenge (one-time use).

---

## 📋 4. Audit Logging Terstruktur (`audit_log.hpp`)

- **Karakteristik**: *Ring buffer* in-memory berkapasitas 10.000 entri tercatat otomatis secara terstruktur.
- **Aksi Tercatat**: `login`, `register`, `delete_message`, `create_group`, `2fa_enable`, `password_change`.
- **Ekspor JSON**: Tersedia melalui REST endpoint terproteksi `/api/admin/audit`.
