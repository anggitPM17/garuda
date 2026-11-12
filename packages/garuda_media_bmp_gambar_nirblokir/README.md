# garuda_media_bmp_gambar_nirblokir

> **Modul:** `BmpGambarNirblokir` • **Kategori:** Audio & Sinyal • **Versi:** `0.1.0`

Varian Nirblokir berkinerja tinggi: Pengurai dan pembaca berkas citra bitmap BMP 24-bit dan 32-bit RGBA.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur nirblokir untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_media_bmp_gambar_nirblokir = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_media_bmp_gambar_nirblokir
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BmpGambarNirblokir

publik fungsi utama(): Bulat {
    konst instansi = BmpGambarNirblokir::inisialisasi()
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