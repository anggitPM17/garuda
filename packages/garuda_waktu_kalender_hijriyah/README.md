# garuda_waktu_kalender_hijriyah

> **Modul:** `KalenderHijriyah` • **Kategori:** Waktu & Kalender • **Versi:** `0.1.0`

Konversi penanggalan masehi ke kalender lunar Hijriyah berbasis hisab astronomis.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Waktu & Kalender.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_waktu_kalender_hijriyah = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_waktu_kalender_hijriyah
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KalenderHijriyah

publik fungsi utama(): Bulat {
    konst instansi = KalenderHijriyah::inisialisasi()
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