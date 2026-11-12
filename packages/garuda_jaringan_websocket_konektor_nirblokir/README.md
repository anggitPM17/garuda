# garuda_jaringan_websocket_konektor_nirblokir

> **Modul:** `WebsocketKonektorNirblokir` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Varian Nirblokir berkinerja tinggi: Protokol WebSocket RFC 6455 full-duplex dengan framing biner dan masking.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur nirblokir untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_websocket_konektor_nirblokir = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_websocket_konektor_nirblokir
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor WebsocketKonektorNirblokir

publik fungsi utama(): Bulat {
    konst instansi = WebsocketKonektorNirblokir::inisialisasi()
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