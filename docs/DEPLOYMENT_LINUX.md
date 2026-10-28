# 🐧 Panduan Deployment Garuda BEAM & RICHAT Server di Linux (VPS)

Dokumen ini menjelaskan langkah-langkah deployment **Garuda BEAM Production Server (`garuda_richat_server`)** di Linux (Ubuntu / Debian VPS) secara mandiri.

---

## 📋 1. Prasyarat System VPS Linux

- **OS:** Ubuntu 22.04 LTS / Debian 12
- **RAM Minimum:** 1 GB (2 GB disarankan)
- **CPU:** 1 Core
- **Port Terbuka:** `9001` (WebSocket), `80/443` (HTTP/HTTPS Nginx Proxy)

---

## 🛠️ 2. Instalsi Dependencies di VPS

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gcc \
    g++ \
    git \
    nginx \
    certbot \
    python3-certbot-nginx
```

---

## 🏗️ 3. Kompilasi Binary Produksi di VPS

```bash
# Clone repository
git clone https://github.com/garuda-org/garuda.git
cd garuda/garuda_v67_0_production/garuda_v67.0.0

# Build menggunakan script Linux
chmod +x scripts/build_linux.sh scripts/run_tests.sh
./scripts/build_linux.sh

# Uji kelayakan seluruh test suite
./scripts/run_tests.sh
```

Binary server produksi berlokasi di:
`build/garuda_richat_server`

---

## ⚙️ 4. Konfigurasi Service Systemd (`richat.service`)

Buat file systemd service:

```bash
sudo nano /etc/systemd/system/richat.service
```

Isi konfigurasi berikut:

```ini
[Unit]
Description=Garuda BEAM RICHAT Production Server
After=network.target

[Service]
Type=simple
User=ubuntu
WorkingDirectory=/home/ubuntu/garuda/garuda_v67_0_production/garuda_v67.0.0
ExecStart=/home/ubuntu/garuda/garuda_v67_0_production/garuda_v67.0.0/build/garuda_richat_server 9001 ./richat_production_data
Restart=always
RestartSec=5
LimitNOFILE=65536

[Install]
WantedBy=multi-user.target
```

Aktifkan service:

```bash
sudo systemctl daemon-reload
sudo systemctl enable richat
sudo systemctl start richat
sudo systemctl status richat
```

---

## 🌐 5. Setup Reverse Proxy Nginx & SSL Certbot

Konfigurasi Nginx (`/etc/nginx/sites-available/richat`):

```nginx
server {
    server_name richat.example.com;

    location / {
        proxy_pass http://127.0.0.1:9001;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "Upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_read_timeout 86400s;
        proxy_send_timeout 86400s;
    }
}
```

Aktifkan Nginx & Certificate:

```bash
sudo ln -s /etc/nginx/sites-available/richat /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx
sudo certbot --nginx -d richat.example.com
```

---

## 📊 6. Monitoring & Logging

- **Cek logs live:** `journalctl -u richat -f`
- **Cek status:** `systemctl status richat`
- **Uptime & Metrics:** Akses via WebSocket `RICHAT_LOGIN` & Prometheus metrics endpoint.
