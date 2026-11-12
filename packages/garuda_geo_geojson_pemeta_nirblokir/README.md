# garuda_geo_geojson_pemeta_nirblokir

> **Modul:** `GeojsonPemetaNirblokir` • **Kategori:** Geospasial & Navigasi • **Versi:** `0.1.0`

Varian Nirblokir berkinerja tinggi: Parser spesifikasi GeoJSON RFC 7946 untuk Point, LineString, dan Polygon.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur nirblokir untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_geo_geojson_pemeta_nirblokir = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_geo_geojson_pemeta_nirblokir
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor GeojsonPemetaNirblokir

publik fungsi utama(): Bulat {
    konst instansi = GeojsonPemetaNirblokir::inisialisasi()
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