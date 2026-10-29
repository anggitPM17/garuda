# 🤖 Garuda BEAM — Autopilot AI Self-Healing Runtime (FASE D.1)

Dokumen ini menjelaskan arsitektur dan cara kerja **Autopilot AI Self-Healing Runtime Engine** pada Garuda BEAM.

---

## 🌟 Keunggulan Khas Garuda BEAM vs Erlang/OTP

Erlang/OTP menggunakan filosofi *"Let it Crash"* dengan Supervision Trees. Garuda BEAM **melangkah lebih jauh** dengan filosofi **"Self-Healing Runtime"**:

- **Anomaly Detection**: Deteksi dini lonjakan CPU, Memory Leak, Error Rate tinggi, atau Crash berulang sebelum kegagalan total.
- **Root Cause Analysis (RCA)**: Mengorelasikan metrik dengan log runtime untuk mengidentifikasi penyebab akar masalah.
- **Action Engine**: Mengusulkan & mengeksekusi tindakan perbaikan (Restart Service, Rollback Config, Alert Admin, Apply Hot Patch) dengan dukungan mode `MANUAL` & `AUTOPILOT`.

---

## 🏗️ Komponen Utama Engine Autopilot AI

```
┌────────────────────────────────────────────────────────────┐
│                    AUTOPILOT RUNTIME                       │
├──────────────────┬──────────────────────┬──────────────────┤
│ AnomalyDetector  │      RcaEngine       │   ActionEngine   │
│ (Ingest Metrics) │ (Log Correlation)    │ (Safety & Mode)  │
└──────────────────┴──────────────────────┴──────────────────┘
```

1. **`AnomalyDetector` (`anomaly_detector.hpp`)**:
   - Mendaftarkan aturan threshold metrik (`cpu_percent`, `memory_percent`, `error_rate`, `crash_count`).
   - Melacak jumlah pelanggaran beruntun (`consecutive_breaches`) sebelum menandai anomali aktif.

2. **`RcaEngine` (`rca_engine.hpp`)**:
   - Memelihara *ring buffer* log peristiwa runtime.
   - Menganalisis pola error dan mengkorelasikannya dengan anomali terdeteksi untuk menghitung tingkat kepercayaan (*confidence score* 0.0 - 1.0) dan usulan solusi.

3. **`ActionEngine` (`action_engine.hpp`)**:
   - Mendukung dua mode operasi:
     - **`MANUAL` (Default)**: Memerlukan persetujuan admin (`approve`/`reject`) sebelum tindakan berbahaya (Restart/Rollback) dieksekusi.
     - **`AUTOPILOT`**: Mengeksekusi otomatis tindakan aman (misal: Notification Alert).

4. **`AutopilotRuntime` (`autopilot_runtime.hpp`)**:
   - Menjalankan siklus *one-shot tick* end-to-end, mencatat seluruh riwayat audit peristiwa (`event_log`), dan merender dashboard status TUI.

---

## 🚀 Penggunaan & Integrasi

```cpp
#include "garuda/autopilot/autopilot_runtime.hpp"

using namespace garuda::autopilot;

AutopilotRuntime runtime;

// 1. Ingest metric & log
runtime.detector().ingest_metric("cpu_percent", 92.0);
runtime.rca().ingest_log("[CRITICAL] CPU spike on worker thread");

// 2. Tentukan mode
runtime.set_mode(ActionMode::MANUAL);

// 3. Jalankan siklus evaluasi
runtime.tick();

// 4. Lihat status dashboard
std::cout << runtime.render_status() << std::endl;
```
