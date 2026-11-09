# garuda_format_base64_url

> **Modul:** `Base64Url` • **Kategori:** Parsing & Format Data • **Versi:** `0.1.0`

Enkoder dan dekoder Base64 standar dan Base64URL aman web tanpa padding.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Parsing & Format Data.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_format_base64_url = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_format_base64_url
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor Base64Url

publik fungsi utama(): Bulat {
    konst instansi = Base64Url::inisialisasi()
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