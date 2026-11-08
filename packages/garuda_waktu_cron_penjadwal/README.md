# garuda_waktu_cron_penjadwal

> **Modul:** `CronPenjadwal` • **Kategori:** Waktu & Kalender • **Versi:** `0.1.0`

Evaluator ekspresi penjadwalan cron 5 bidang untuk pemicu tugas berkala.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Waktu & Kalender.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_waktu_cron_penjadwal = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_waktu_cron_penjadwal
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor CronPenjadwal

publik fungsi utama(): Bulat {
    konst instansi = CronPenjadwal::inisialisasi()
    kembalikan 0
}
```

## Batasan yang Diketahui (Known Limitations)
- Modul ini dirancang murni di atas runtime GVM (Garuda Virtual Machine).
- Bergantung pada ketersediaan alokasi heap memori thread-safe.

## Kebutuhan Engine C++ GVM
- Jika modul memerlukan operasi I/O jaringan fisik langsung, modul memanggil FFI socket `garuda_socket_ffi`.
- Kriptografi CSPRNG membutuhkan entropy source `/dev/urandom` atau WinCrypt pada level host C++.

## Lisensi
Lisensi MIT © 2026 Anggit Prasetyo (@anggitPM17).