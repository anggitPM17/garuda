# garuda_algoritma_rbtree_pohon_nirblokir

> **Modul:** `RbtreePohonNirblokir` • **Kategori:** Struktur Data & Algoritma • **Versi:** `0.1.0`

Varian Nirblokir berkinerja tinggi: Pohon merah-hitam untuk indeks tabel dan pemetaan terurut berkinerja tinggi.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur nirblokir untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_algoritma_rbtree_pohon_nirblokir = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_algoritma_rbtree_pohon_nirblokir
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor RbtreePohonNirblokir

publik fungsi utama(): Bulat {
    konst instansi = RbtreePohonNirblokir::inisialisasi()
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