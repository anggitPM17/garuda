# garuda_format_cbor_biner_nirblokir

> **Modul:** `CborBinerNirblokir` • **Kategori:** Parsing & Format Data • **Versi:** `0.1.0`

Varian Nirblokir berkinerja tinggi: Concise Binary Object Representation RFC 8949 untuk aplikasi IoT terdistribusi.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur nirblokir untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_format_cbor_biner_nirblokir = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_format_cbor_biner_nirblokir
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor CborBinerNirblokir

publik fungsi utama(): Bulat {
    konst instansi = CborBinerNirblokir::inisialisasi()
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