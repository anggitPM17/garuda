# garuda_algoritma_splay_pohon

> **Modul:** `SplayPohon` • **Kategori:** Struktur Data & Algoritma • **Versi:** `0.1.0`

Pohon pencarian biner adaptif mandiri yang menaikkan elemen sering diakses ke akar.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Struktur Data & Algoritma.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_algoritma_splay_pohon = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_algoritma_splay_pohon
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor SplayPohon

publik fungsi utama(): Bulat {
    konst instansi = SplayPohon::inisialisasi()
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