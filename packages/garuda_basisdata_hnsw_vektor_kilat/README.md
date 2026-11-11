# garuda_basisdata_hnsw_vektor_kilat

> **Modul:** `HnswVektorKilat` • **Kategori:** Penyimpanan & Basis Data • **Versi:** `0.1.0`

Varian Kilat berkinerja tinggi: Indeks graf Hierarchical Navigable Small World untuk pencarian vektor kemiripan AI.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur kilat untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_basisdata_hnsw_vektor_kilat = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_basisdata_hnsw_vektor_kilat
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor HnswVektorKilat

publik fungsi utama(): Bulat {
    konst instansi = HnswVektorKilat::inisialisasi()
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