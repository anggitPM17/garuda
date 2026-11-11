# garuda_kecerdasan_knn_tetangga_kilat

> **Modul:** `KnnTetanggaKilat` • **Kategori:** Pembelajaran Mesin & AI • **Versi:** `0.1.0`

Varian Kilat berkinerja tinggi: Klasifikasi k-nearest neighbors dengan penghitungan jarak Euclidean dan Manhattan.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur kilat untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kecerdasan_knn_tetangga_kilat = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kecerdasan_knn_tetangga_kilat
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KnnTetanggaKilat

publik fungsi utama(): Bulat {
    konst instansi = KnnTetanggaKilat::inisialisasi()
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