# garuda_game_partikel_efek

> **Modul:** `GamePartikelEfek` • **Kategori:** Pengembangan Game • **Versi:** `0.1.0`

Sistem Particle Emitter 2D/3D untuk efek visual dinamis seperti api, asap, ledakan, percikan listrik, hujan, salju, dan sihir magis.

## Arsitektur & Tujuan
Efek visual kinetik (VFX) game berkinerja tinggi dengan memory recycling object pool.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_game_partikel_efek = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_game_partikel_efek
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GamePartikelEfek

publik fungsi utama(): Bulat {
    konst instansi = GamePartikelEfek::inisialisasi()
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