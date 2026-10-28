# 🦅 Garuda BEAM (G-BEAM) 🇮🇩

[![Build Status](https://github.com/anggitPM17/garuda/actions/workflows/ci-linux.yml/badge.svg)](https://github.com/anggitPM17/garuda/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Production](https://img.shields.io/badge/Production-Live-success)](https://richat.my.id)
[![RAM Usage](https://img.shields.io/badge/RAM-536%20KB-brightgreen)](https://richat.my.id)

> Runtime terdistribusi setara Erlang/OTP — dengan identitas Indonesia.
> C++ header-only. Zero external dependencies (kecuali mbedTLS & SQLite3).

---

## 🌐 Live Production

- **URL Production**: https://richat.my.id
- **Status**: ✅ **Live & Primary Backend** (Garuda BEAM C++ Server)
- **Runtime RAM Usage**: **~536 KB - 1.8 MB** (vs Erlang ~69.4 MB — **38x lebih hemat**)
- **Stress Capability**: 191 RPS, 100% availability, 0 failed requests
- **Auto-Recovery**: Systemd auto-restart < 5 detik

---

## ✨ Fitur Lengkap (8 Fase + Port Linux, 100% PASS)

| Fase | Fokus | Status |
|---|---|---|
| **G1** | OTP Behaviours (GenServer, Supervisor, Application, PubSub, GenStatem) | ✅ |
| **G2** | Distributed Nodes (DistNode, GlobalRegistry, ClusterManager, PG) | ✅ |
| **G3** | Mnesia Setara (Core, Tier, Sweep, Cluster) | ✅ |
| **G4** | Crypto (SHA256, Merkle Tree, Sealed Sender, E2E Session) | ✅ |
| **G5** | WebRTC & Presence (Signaling, Presence, Typing, Receipt, Reaction) | ✅ |
| **G6** | Shell & REPL (Shell, RPC, Observer) | ✅ |
| **G7** | Telemetry (Metrics, Tracing, Logger, Aggregator) | ✅ |
| **G8** | Application Layer RICHAT (App, GroupMember, Fanout, Dist, ColdDB) | ✅ |
| **B**  | Port Linux & Cross-Platform (Build Scripts, CI, Docker, Deployment Guide) | ✅ |

---

## 🚀 Build & Run

### Linux / macOS
```bash
./scripts/build_linux.sh
./scripts/run_tests.sh
```

### Windows (MSVC / Ninja)
```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
powershell -ExecutionPolicy Bypass -File scripts/run_tests.ps1
```

### Docker Container
```bash
docker build -t garuda-beam .
docker run --rm -p 9002:9002 garuda-beam
```

---

## 📊 Benchmark & Production Metrics

- **HTTP Flood**: 191 req/s, 0 failed (di VPS 1 vCPU, 1GB RAM)
- **1000 User Load**: 2000 transaksi, 100% availability
- **RAM Memory Usage**: **~1.8 MB** (vs Erlang ~69.4 MB — **38x lebih hemat**)
- **Chaos (SIGKILL)**: Auto-restart < 5 detik (systemd)
- **Security & Robustness**: Path traversal & SQL injection blocked

---

## 📁 Struktur Directory

```
include/garuda/
├── beam/           # Runtime inti
│   ├── core/       # Scheduler, Actor, GC, Term
│   ├── otp/        # GenServer, Supervisor, Mnesia, dll
│   ├── dist/       # Distributed Nodes
│   ├── crypto/     # Crypto & E2E
│   ├── webrtc/     # WebRTC & Presence
│   ├── shell/      # Shell, RPC, Observer
│   └── telemetry/  # Metrics, Tracing, Logger
├── richat/         # Application Layer RICHAT
└── jalur_langit/   # Database Engine

tests/              # 50+ test executable, semua PASS
scripts/            # Build & test scripts
docs/               # Panduan deployment Linux VPS
```

---

## 📚 Dokumentasi

- [Panduan Deployment Linux VPS](docs/DEPLOYMENT_LINUX.md)
- [Roadmap Garuda BEAM](ROADMAP_GARUDA_BEAM.md)

---

## 🏆 Pencapaian

- ✅ 8 fase + Port Linux, 35+ sub-fase, 50+ test executables, semua PASS
- ✅ Production-tested & deployed di VPS Ubuntu Linux (`https://richat.my.id`)
- ✅ 38x lebih hemat RAM dibanding Erlang
- ✅ Zero downtime migration & parallel fallback architecture

---

## 📜 Lisensi

[MIT](LICENSE) — bebas digunakan, dimodifikasi, dan didistribusikan.

---

## 🇮🇩 Tentang

Garuda BEAM adalah runtime terdistribusi berkinerja tinggi karya bangsa.
Dibuat dengan visi: **"Erlang/OTP versi Indonesia, dengan identitas dan efisiensi memori yang jauh lebih baik."**

---
**Dibuat dengan ❤️ di Indonesia. 🦅**
