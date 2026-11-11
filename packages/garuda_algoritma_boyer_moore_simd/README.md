# garuda_algoritma_boyer_moore_simd

> **Modul:** `BoyerMooreSimd` • **Kategori:** Struktur Data & Algoritma • **Versi:** `0.1.0`

Varian Simd berkinerja tinggi: Pencarian string berkecepatan tinggi dengan aturan bad-character dan good-suffix.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur simd untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_algoritma_boyer_moore_simd = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_algoritma_boyer_moore_simd
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BoyerMooreSimd

publik fungsi utama(): Bulat {
    konst instansi = BoyerMooreSimd::inisialisasi()
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