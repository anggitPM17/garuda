# garuda_kripto_ed25519_tanda

> **Modul:** `Ed25519Tanda` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

Tanda tangan digital kurva eliptik berkecepatan tinggi dan tahan side-channel.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Keamanan & Kriptografi.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto_ed25519_tanda = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto_ed25519_tanda
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Ed25519Tanda

publik fungsi utama(): Bulat {
    konst instansi = Ed25519Tanda::inisialisasi()
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