# garuda_kripto

> **Modul:** `Kripto` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

Kriptografi standar SHA-256 (ekspansi/kompresi 64 putaran), HMAC-SHA256, komparasi waktu-konstan, dan CSPRNG binding.

## Arsitektur & Tujuan
Fondasi integritas data, tanda tangan digital, dan keamanan transmisi protokol jaringan.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Kripto

publik fungsi utama(): Bulat {
    konst instansi = Kripto::inisialisasi()
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