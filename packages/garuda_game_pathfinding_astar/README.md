# garuda_game_pathfinding_astar

> **Modul:** `GamePathfindingAstar` • **Kategori:** Pengembangan Game • **Versi:** `0.1.0`

Navigasi kecerdasan buatan jalur terpendek (A* Pathfinding) pada Grid 2D/3D dan Navmesh dengan penghitungan heuristik Manhattan/Euclidean dan penghindaran rintangan.

## Arsitektur & Tujuan
Penemuan rute otomatis untuk musuh, unit RTS, dan navigasi karakter di dunia game kompleks.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_game_pathfinding_astar = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_game_pathfinding_astar
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GamePathfindingAstar

publik fungsi utama(): Bulat {
    konst instansi = GamePathfindingAstar::inisialisasi()
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