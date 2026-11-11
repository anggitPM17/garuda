# garuda_kripto_argon2_hash_simd

> **Modul:** `Argon2HashSimd` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

Varian Simd berkinerja tinggi: Fungsi derivasi kunci pemenang kompetisi hashing kata sandi tahan serangan GPU/ASIC.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur simd untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto_argon2_hash_simd = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto_argon2_hash_simd
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Argon2HashSimd

publik fungsi utama(): Bulat {
    konst instansi = Argon2HashSimd::inisialisasi()
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