# garuda_kripto_ed25519_tanda_vektor

> **Modul:** `Ed25519TandaVektor` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

Varian Vektor berkinerja tinggi: Tanda tangan digital kurva eliptik berkecepatan tinggi dan tahan side-channel.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur vektor untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto_ed25519_tanda_vektor = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto_ed25519_tanda_vektor
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Ed25519TandaVektor

publik fungsi utama(): Bulat {
    konst instansi = Ed25519TandaVektor::inisialisasi()
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