# garuda_game_ui_hud

> **Modul:** `GameUiHud` • **Kategori:** Pengembangan Game • **Versi:** `0.1.0`

Komponen UI Dalam Game (Heads-Up Display): Bar Darah/HP, Bar Mana, Papan Skor/Leaderboard, Inventaris Item Grid, dan Kotak Dialog NPC.

## Arsitektur & Tujuan
Antarmuka pemain interaktif siap pakai untuk visualisasi status petualangan dan statistik permainan.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_game_ui_hud = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_game_ui_hud
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GameUiHud

publik fungsi utama(): Bulat {
    konst instansi = GameUiHud::inisialisasi()
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