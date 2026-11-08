# garuda_geo_geohash_kalkulator

> **Modul:** `GeohashKalkulator` • **Kategori:** Geospasial & Navigasi • **Versi:** `0.1.0`

Pengkodean koordinat lintang/bujur menjadi string geohash dan bounding box.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Geospasial & Navigasi.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_geo_geohash_kalkulator = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_geo_geohash_kalkulator
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GeohashKalkulator

publik fungsi utama(): Bulat {
    konst instansi = GeohashKalkulator::inisialisasi()
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