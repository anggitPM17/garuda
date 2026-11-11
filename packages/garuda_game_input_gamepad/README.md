# garuda_game_input_gamepad

> **Modul:** `GameInputGamepad` • **Kategori:** Pengembangan Game • **Versi:** `0.1.0`

Pengelola input Virtual Joystick / D-Pad layar sentuh mobile, serta abstraksi pemetaan tombol Keyboard, Mouse, dan Gamepad / Controller USB/Bluetooth.

## Arsitektur & Tujuan
Kontroler terpadu multi-platform (Android, Desktop, Web) dengan deadzone filter.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_game_input_gamepad = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_game_input_gamepad
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GameInputGamepad

publik fungsi utama(): Bulat {
    konst instansi = GameInputGamepad::inisialisasi()
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