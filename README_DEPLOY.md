# 🔑 PANDUAN SETUP GITHUB SECRETS & CI/CD DEPLOYMENT GVM

Dokumen ini berisi panduan untuk mengaktifkan **CI/CD Auto-Deploy** dan **CodeQL Security Scan** pada repositori GitHub **[anggitPM17/garuda](https://github.com/anggitPM17/garuda)**.

---

## 📋 Langkah Setup GitHub Secrets (3 Kunci)

Buka repositori GitHub Anda: **`https://github.com/anggitPM17/garuda`**
Pergi ke menu: **Settings** → **Secrets and variables** → **Actions** → Klik **New repository secret**.

Tambahkan 3 secret berikut:

### 1. `VPS_HOST`
* **Name**: `VPS_HOST`
* **Secret Value**: `36.50.77.230`

### 2. `VPS_USER`
* **Name**: `VPS_USER`
* **Secret Value**: `root`

### 3. `VPS_SSH_KEY`
* **Name**: `VPS_SSH_KEY`
* **Secret Value**: Isi dengan **Private Key SSH** untuk akses VPS tanpa password.

---

## 🛠️ Cara Membuat SSH Deploy Key (Di Terminal VPS / Laptop)

Jika Anda belum memiliki SSH Private Key khusus untuk GitHub Actions, buat pasangan SSH key baru:

```bash
# 1. Generate SSH Key Pair (Ed25519)
ssh-keygen -t ed25519 -C "github-actions-deploy" -f ~/.ssh/gvm_deploy_key -N ""

# 2. Tambahkan Public Key ke authorized_keys di VPS
cat ~/.ssh/gvm_deploy_key.pub >> ~/.ssh/authorized_keys
chmod 600 ~/.ssh/authorized_keys

# 3. Tampilkan Private Key (Salin SELURUH output ini ke GitHub Secret: VPS_SSH_KEY)
cat ~/.ssh/gvm_deploy_key
```

> **⚠️ Penting:** Pastikan menyalin dari baris `-----BEGIN OPENSSH PRIVATE KEY-----` sampai `-----END OPENSSH PRIVATE KEY-----` termasuk enter baris baru.

---

## 🚀 Alur Kerja CI/CD

1. **Pull Request / Commit Push**:
   * Pipelined job `build-and-test` akan mengompilasi `garuda_richat_server` dan `gvm-admin` di runner Ubuntu Linux.
   * `codeql.yml` menjalankan static security analysis untuk mendeteksi potensi vulnerability C++.
2. **Merge ke Branch `main`**:
   * Pipelined job `deploy` memicu SSH connection ke VPS `36.50.77.230`.
   * Menjalankan `git pull`, kompilasi otomatis, pemasangan `gvm-admin`, dan `systemctl restart richat-garuda`.
