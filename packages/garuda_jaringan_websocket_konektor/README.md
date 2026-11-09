# garuda_jaringan_websocket_konektor

> **Modul:** `WebsocketKonektor` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Protokol WebSocket RFC 6455 full-duplex dengan framing biner dan masking.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Jaringan & Protokol.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_websocket_konektor = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_websocket_konektor
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor WebsocketKonektor

publik fungsi utama(): Bulat {
    konst instansi = WebsocketKonektor::inisialisasi()
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