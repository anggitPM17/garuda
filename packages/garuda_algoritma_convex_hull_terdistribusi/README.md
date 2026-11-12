# garuda_algoritma_convex_hull_terdistribusi

> **Modul:** `ConvexHullTerdistribusi` • **Kategori:** Struktur Data & Algoritma • **Versi:** `0.1.0`

Varian Terdistribusi berkinerja tinggi: Algoritma Graham Scan dan Monotone Chain untuk batas cembung kumpulan titik.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur terdistribusi untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_algoritma_convex_hull_terdistribusi = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_algoritma_convex_hull_terdistribusi
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor ConvexHullTerdistribusi

publik fungsi utama(): Bulat {
    konst instansi = ConvexHullTerdistribusi::inisialisasi()
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