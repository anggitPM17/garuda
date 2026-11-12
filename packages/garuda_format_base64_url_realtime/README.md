# garuda_format_base64_url_realtime

> **Modul:** `Base64UrlRealtime` • **Kategori:** Parsing & Format Data • **Versi:** `0.1.0`

Varian Realtime berkinerja tinggi: Enkoder dan dekoder Base64 standar dan Base64URL aman web tanpa padding.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur realtime untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_format_base64_url_realtime = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_format_base64_url_realtime
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Base64UrlRealtime

publik fungsi utama(): Bulat {
    konst instansi = Base64UrlRealtime::inisialisasi()
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