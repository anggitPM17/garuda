# garuda_algoritma_kmp_pencari

> **Modul:** `KmpPencari` • **Kategori:** Struktur Data & Algoritma • **Versi:** `0.1.0`

Pencarian substring Knuth-Morris-Pratt waktu linier dengan tabel awalan parsial.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Struktur Data & Algoritma.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_algoritma_kmp_pencari = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_algoritma_kmp_pencari
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KmpPencari

publik fungsi utama(): Bulat {
    konst instansi = KmpPencari::inisialisasi()
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