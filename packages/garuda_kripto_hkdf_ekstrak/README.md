# garuda_kripto_hkdf_ekstrak

> **Modul:** `HkdfEkstrak` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

HMAC-based Extract-and-Expand Key Derivation Function sesuai RFC 5869.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Keamanan & Kriptografi.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto_hkdf_ekstrak = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto_hkdf_ekstrak
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor HkdfEkstrak

publik fungsi utama(): Bulat {
    konst instansi = HkdfEkstrak::inisialisasi()
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