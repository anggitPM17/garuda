# garuda_kompresi_gzip_kompresor_nirblokir

> **Modul:** `GzipKompresorNirblokir` • **Kategori:** Kompresi & Berkas • **Versi:** `0.1.0`

Varian Nirblokir berkinerja tinggi: Kompresi dan dekompresi format berkas GZIP RFC 1952 dengan checksum CRC32.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur nirblokir untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompresi_gzip_kompresor_nirblokir = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompresi_gzip_kompresor_nirblokir
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GzipKompresorNirblokir

publik fungsi utama(): Bulat {
    konst instansi = GzipKompresorNirblokir::inisialisasi()
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