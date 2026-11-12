# garuda_algoritma_kmp_pencari_terenkripsi

> **Modul:** `KmpPencariTerenkripsi` • **Kategori:** Struktur Data & Algoritma • **Versi:** `0.1.0`

Varian Terenkripsi berkinerja tinggi: Pencarian substring Knuth-Morris-Pratt waktu linier dengan tabel awalan parsial.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur terenkripsi untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_algoritma_kmp_pencari_terenkripsi = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_algoritma_kmp_pencari_terenkripsi
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KmpPencariTerenkripsi

publik fungsi utama(): Bulat {
    konst instansi = KmpPencariTerenkripsi::inisialisasi()
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