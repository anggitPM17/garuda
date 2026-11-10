# garuda_webmodern_middleware_pipeline

> **Modul:** `MiddlewarePipeline` • **Kategori:** Pengembangan Website Modern • **Versi:** `0.1.0`

Rantai pemrosesan request HTTP mencakup CORS, Helmet security headers, Gzip/Brotli, dan rate limiter.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Pengembangan Website Modern.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_webmodern_middleware_pipeline = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_webmodern_middleware_pipeline
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor MiddlewarePipeline

publik fungsi utama(): Bulat {
    konst instansi = MiddlewarePipeline::inisialisasi()
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