# garuda_robotik_kontroler_pid

> **Modul:** `KontrolerPid` • **Kategori:** Robotik & Sistem Neurolink • **Versi:** `0.1.0`

Pengendali lup tertutup PID sub-milidetik dengan anti-windup untuk motor servo presisi.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Robotik & Sistem Neurolink.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_robotik_kontroler_pid = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_robotik_kontroler_pid
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KontrolerPid

publik fungsi utama(): Bulat {
    konst instansi = KontrolerPid::inisialisasi()
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