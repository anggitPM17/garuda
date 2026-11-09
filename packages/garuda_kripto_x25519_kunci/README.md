# garuda_kripto_x25519_kunci

> **Modul:** `X25519Kunci` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

Protokol pertukaran kunci Diffie-Hellman efisien berbasis Curve25519.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Keamanan & Kriptografi.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto_x25519_kunci = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto_x25519_kunci
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor X25519Kunci

publik fungsi utama(): Bulat {
    konst instansi = X25519Kunci::inisialisasi()
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