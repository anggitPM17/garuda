# garuda_grafika_bezier_kurva_cepat

> **Modul:** `BezierKurvaCepat` • **Kategori:** Matematika & Grafika • **Versi:** `0.1.0`

Varian Cepat berkinerja tinggi: Perhitungan kurva Bezier kuadratik dan kubik untuk animasi dan pemodelan jalur.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur cepat untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_grafika_bezier_kurva_cepat = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_grafika_bezier_kurva_cepat
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BezierKurvaCepat

publik fungsi utama(): Bulat {
    konst instansi = BezierKurvaCepat::inisialisasi()
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