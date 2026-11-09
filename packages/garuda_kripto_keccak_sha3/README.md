# garuda_kripto_keccak_sha3

> **Modul:** `KeccakSha3` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

Fungsi hash kriptografis keluarga SHA-3 berbasis sponge construction.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Keamanan & Kriptografi.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto_keccak_sha3 = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto_keccak_sha3
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KeccakSha3

publik fungsi utama(): Bulat {
    konst instansi = KeccakSha3::inisialisasi()
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