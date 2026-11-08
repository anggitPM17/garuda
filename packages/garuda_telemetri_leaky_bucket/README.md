# garuda_telemetri_leaky_bucket

> **Modul:** `LeakyBucket` • **Kategori:** Telemetri & Observabilitas • **Versi:** `0.1.0`

Penghalus lonjakan lalu lintas data jaringan menggunakan algoritma keranjang bocor.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Telemetri & Observabilitas.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_telemetri_leaky_bucket = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_telemetri_leaky_bucket
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor LeakyBucket

publik fungsi utama(): Bulat {
    konst instansi = LeakyBucket::inisialisasi()
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