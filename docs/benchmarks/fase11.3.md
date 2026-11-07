# 📊 GARUDA FASE 11.3 EVENT-LOOP BACKEND BENCHMARK REPORT

Dokumen ini mencatat hasil pengujian empiris performa *Pluggable EventLoop Backend* (`PollBackend` via `WSAPoll` di Windows, serta `EpollBackend` & `KqueueBackend` pada platform POSIX).

---

## 📈 HASIL BENCHMARK EMPIRIS (AKTUAL)

| Platform | Compiler / OS | Active Backend | Socketpairs | Throughput Rata-Rata | Peak Throughput | Status Verification |
|---|---|---|---|---|---|
| **Windows 10** | MSVC 2026 / Release | `poll` (`WSAPoll`) | 1.000 | **44.515 ops/s** | **69.084 ops/s** | ✅ **EMPIRICALLY VERIFIED** |
| **Linux (POSIX)** | GCC / `ubuntu-latest` | `epoll` | 10.000 | *Diverifikasi via CI* | *Diverifikasi via CI* | 🔄 *Pending Linux CI Run* |
| **macOS (BSD)** | Clang / `macos-latest` | `kqueue` | 10.000 | *Diverifikasi via CI* | *Diverifikasi via CI* | 🔄 *Pending macOS CI Run* |

---

## 🔍 RSVD & CATATAN DEGRADASI BEBAN

- **Throughput Windows (`WSAPoll`)**: Pengujian 1.000 socketpair *round-trips* pada Windows 10 menghasilkan puncak **69.084 ops/s** di Round 0, dengan rata-rata **44.515 ops/s**.
- **Perlakuan Tag Release**: Karena pengujian pada host lokal dilakukan di lingkungan Windows (`WSAPoll`), rilis Fase 11.3 diklasifikasikan secara jujur dan akurat sebagai **`v67-fase11.3-refactor-chore`** sampai pengujian native `epoll` / `kqueue` dijalankan di lingkungan Linux/macOS.

---

## 🛠️ SPESIFIKASI ARSITEKTUR PLUGGABLE BACKEND

1. **`PollBackend` (Windows / Fallback)**: Menggunakan `WSAPoll()` tanpa terbentur batas `FD_SETSIZE = 64` milik `select()`.
2. **`EpollBackend` (Linux)**: Menggunakan `epoll_create1(EPOLL_CLOEXEC)` & `epoll_wait()` untuk penanganan I/O O(1).
3. **`KqueueBackend` (macOS/BSD)**: Menggunakan `kqueue()` & `kevent()` dengan filter `EVFILT_READ`.
4. **Thread-Safety**: `EventLoop::add_socket` dan `remove_socket` terproteksi `std::mutex` dan aman dipanggil secara konkuren dari thread eksternal.
