# garuda_webmodern_wasm_interop_bridge

> **Modul:** `WasmInteropBridge` • **Kategori:** Pengembangan Website Modern • **Versi:** `0.1.0`

Jembatan kompilasi dan interop WebAssembly (Wasm) untuk komputasi berat di peramban.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Pengembangan Website Modern.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_webmodern_wasm_interop_bridge = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_webmodern_wasm_interop_bridge
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor WasmInteropBridge

publik fungsi utama(): Bulat {
    konst instansi = WasmInteropBridge::inisialisasi()
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