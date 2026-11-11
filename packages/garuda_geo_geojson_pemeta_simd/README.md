# garuda_geo_geojson_pemeta_simd

> **Modul:** `GeojsonPemetaSimd` • **Kategori:** Geospasial & Navigasi • **Versi:** `0.1.0`

Varian Simd berkinerja tinggi: Parser spesifikasi GeoJSON RFC 7946 untuk Point, LineString, dan Polygon.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur simd untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_geo_geojson_pemeta_simd = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_geo_geojson_pemeta_simd
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GeojsonPemetaSimd

publik fungsi utama(): Bulat {
    konst instansi = GeojsonPemetaSimd::inisialisasi()
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