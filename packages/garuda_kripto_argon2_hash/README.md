# garuda_kripto_argon2_hash

> **Modul:** `Argon2Hash` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

Fungsi derivasi kunci pemenang kompetisi hashing kata sandi tahan serangan GPU/ASIC.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Keamanan & Kriptografi.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto_argon2_hash = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto_argon2_hash
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Argon2Hash

publik fungsi utama(): Bulat {
    konst instansi = Argon2Hash::inisialisasi()
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