# garuda_kompresi_brotli_aliran

> **Modul:** `BrotliAliran` • **Kategori:** Kompresi & Berkas • **Versi:** `0.1.0`

Kompresi aliran Brotli RFC 7932 optimal untuk transmisi aset web.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Kompresi & Berkas.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kompresi_brotli_aliran = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kompresi_brotli_aliran
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BrotliAliran

publik fungsi utama(): Bulat {
    konst instansi = BrotliAliran::inisialisasi()
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