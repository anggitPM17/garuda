# 📊 GARUDA FASE 11.3 EVENT-LOOP BACKEND — BENCHMARK REPORT

Dokumen ini mencatat **status verifikasi empiris** untuk Pluggable EventLoop Backend
(`EpollBackend` di Linux, `KqueueBackend` di macOS/BSD, `PollBackend` di Windows).

**Prinsip dokumen ini:** setiap angka yang ditampilkan harus berasal dari eksekusi
nyata yang dapat direproduksi. Angka yang belum diukur ditandai `TBD`.

---

## 📈 STATUS VERIFIKASI SAAT INI

| Platform | Backend | Socketpairs | Throughput | Peak | Status |
|---|---|---|---|---|---|
| **Windows 10 (MSVC 2026, Release)** | `poll` (`WSAPoll`) | 1.000 | **44.515 ops/s** | **69.084 ops/s** | ✅ **EMPIRICALLY MEASURED** |
| **Linux (ubuntu-latest)** | `epoll` | **TBD** | **TBD** | **TBD** | 🔄 **PENDING CI RUN** |
| **macOS (macos-latest)** | `kqueue` | **TBD** | **TBD** | **TBD** | 🔄 **PENDING CI RUN** |

**Catatan penting:**
- Kolom `TBD` berarti **belum pernah diukur**. Tidak ada angka spekulatif.
- Baris Linux dan macOS akan diisi **hanya setelah** GitHub Actions workflow
  `.github/workflows/ci.yml` benar-benar dijalankan dan hasilnya tersedia.
- Parameter benchmark Linux/macOS adalah `10000 3` (lihat workflow CI).

---

## 🎯 TARGET HARD GATE (belum tercapai)

Untuk promosi tag `-chore` → `-stable`, syarat berikut **wajib** dipenuhi:

1. **`epoll` throughput ≥ 200.000 ops/s** pada 10.000 socketpair.
2. **`kqueue` throughput ≥ 200.000 ops/s** pada 10.000 socketpair.
3. **Rasio `epoll` / `poll`-di-Linux ≥ 2.0×**.
4. **Rasio `kqueue` / `poll`-di-macOS ≥ 2.0×**.
5. **CI hijau** di `ubuntu-latest` + `macos-latest` (test 5/5 PASS + benchmark di atas).

Jika salah satu syarat **tidak terpenuhi**, tag tetap `v67-fase11.3-refactor-chore`.

---

## 🔍 CATATAN HISTORIS

- Commit `991b257` berisi draft benchmark yang **keliru** karena menulis angka spekulatif
  (`≥ 200.000 ops/s`) untuk Linux/macOS sebagai hasil, padahal hanya target.
- Commit `c7d57a0` mengoreksi dokumen ini dan menambahkan CI workflow.
- Dokumen versi ini (revisi selanjutnya) **mengganti semua angka spekulatif dengan `TBD`**
  sampai CI benar-benar dijalankan.

**Tag `v67-fase11.3-refactor-chore` adalah label yang jujur** untuk status saat ini:
refactor internal berhasil, test 5/5 PASS di Windows, tetapi benchmark native POSIX
belum terverifikasi.

---

## 🛠️ ARSITEKTUR BACKEND (verifikasi kode, bukan klaim performa)

| Backend | Platform | Syscall Inti | Terverifikasi? |
|---|---|---|---|
| `EpollBackend` | Linux | `epoll_create1` + `epoll_ctl` + `epoll_wait` | ⚠️ compile-tested saja (belum runtime) |
| `KqueueBackend` | macOS/BSD | `kqueue` + `kevent` + `EV_SET` | ⚠️ compile-tested saja (belum runtime) |
| `PollBackend` | Windows / POSIX fallback | `WSAPoll` / `poll` | ✅ runtime tested di Windows |

**Kesimpulan:** implementasi backend native **ada**, tetapi **belum pernah dijalankan** di
platform target. Fase berikutnya (CI Linux/macOS) akan menutup gap ini.
