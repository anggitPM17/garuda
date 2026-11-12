# garuda_geo_poligon_irisan_sinaptik

> **Modul:** `PoligonIrisanSinaptik` • **Kategori:** Geospasial & Navigasi • **Versi:** `0.1.0`

Varian Sinaptik berkinerja tinggi: Algoritma Sutherland-Hodgman untuk pemotongan dan uji titik dalam poligon (PIP).

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur sinaptik untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_geo_poligon_irisan_sinaptik = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_geo_poligon_irisan_sinaptik
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor PoligonIrisanSinaptik

publik fungsi utama(): Bulat {
    konst instansi = PoligonIrisanSinaptik::inisialisasi()
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