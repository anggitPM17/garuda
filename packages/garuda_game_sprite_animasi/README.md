# garuda_game_sprite_animasi

> **Modul:** `GameSpriteAnimasi` • **Kategori:** Pengembangan Game • **Versi:** `0.1.0`

Pengelola sprite sheet animation frame, flipbook, kontrol kecepatan loop, dan mesin transisi status animasi karakter (idle, run, jump, attack).

## Arsitektur & Tujuan
Sistem kontrol visual karakter dan entitas animasi 2D yang mulus.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_game_sprite_animasi = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_game_sprite_animasi
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GameSpriteAnimasi

publik fungsi utama(): Bulat {
    konst instansi = GameSpriteAnimasi::inisialisasi()
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