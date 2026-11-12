# garuda_basisdata_hnsw_vektor_submikro

> **Modul:** `HnswVektorSubmikro` • **Kategori:** Penyimpanan & Basis Data • **Versi:** `0.1.0`

Varian Submikro berkinerja tinggi: Indeks graf Hierarchical Navigable Small World untuk pencarian vektor kemiripan AI.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur submikro untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_basisdata_hnsw_vektor_submikro = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_basisdata_hnsw_vektor_submikro
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor HnswVektorSubmikro

publik fungsi utama(): Bulat {
    konst instansi = HnswVektorSubmikro::inisialisasi()
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