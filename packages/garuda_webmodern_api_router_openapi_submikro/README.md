# garuda_webmodern_api_router_openapi_submikro

> **Modul:** `ApiRouterOpenapiSubmikro` • **Kategori:** Pengembangan Website Modern • **Versi:** `0.1.0`

Varian Submikro berkinerja tinggi: Router API RESTful dan JSON-RPC bertipe aman dengan auto-generator spesifikasi OpenAPI 3.1.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur submikro untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_webmodern_api_router_openapi_submikro = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_webmodern_api_router_openapi_submikro
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor ApiRouterOpenapiSubmikro

publik fungsi utama(): Bulat {
    konst instansi = ApiRouterOpenapiSubmikro::inisialisasi()
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