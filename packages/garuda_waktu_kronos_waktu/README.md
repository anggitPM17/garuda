# garuda_waktu_kronos_waktu

> **Modul:** `KronosWaktu` • **Kategori:** Waktu & Kalender • **Versi:** `0.1.0`

Representasi waktu instan nano-detik dengan parsing standar ISO-8601.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Waktu & Kalender.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_waktu_kronos_waktu = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_waktu_kronos_waktu
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KronosWaktu

publik fungsi utama(): Bulat {
    konst instansi = KronosWaktu::inisialisasi()
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