# garuda_game_tilemap_2d

> **Modul:** `GameTilemap2d` • **Kategori:** Pengembangan Game • **Versi:** `0.1.0`

Parser format peta Tiled JSON, rendering layer tile 2D (ortogonal/isometrik), dan deteksi tile bersolid (dinding/lantai/rintangan).

## Arsitektur & Tujuan
Komponen level editor game platformer, RPG, dan top-down 2D di runtime Garuda Game Engine.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_game_tilemap_2d = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_game_tilemap_2d
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GameTilemap2d

publik fungsi utama(): Bulat {
    konst instansi = GameTilemap2d::inisialisasi()
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