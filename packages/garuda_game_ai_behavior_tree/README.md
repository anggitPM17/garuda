# garuda_game_ai_behavior_tree

> **Modul:** `GameAiBehaviorTree` • **Kategori:** Pengembangan Game • **Versi:** `0.1.0`

Kecerdasan Buatan (AI) Musuh berbasis Behavior Tree: composite nodes (Sequence, Selector), Blackboard data sharing, Leaf actions, dan Decorators.

## Arsitektur & Tujuan
Pembuatan logika musuh, NPC, dan bos game yang cerdas, adaptif, dan mudah didebug.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_game_ai_behavior_tree = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_game_ai_behavior_tree
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GameAiBehaviorTree

publik fungsi utama(): Bulat {
    konst instansi = GameAiBehaviorTree::inisialisasi()
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