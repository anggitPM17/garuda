# garuda_algoritma_interval_pohon

> **Modul:** `IntervalPohon` • **Kategori:** Struktur Data & Algoritma • **Versi:** `0.1.0`

Pohon pencarian tumpang-tindih rentang waktu dan penjadwalan alokasi.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Struktur Data & Algoritma.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_algoritma_interval_pohon = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_algoritma_interval_pohon
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor IntervalPohon

publik fungsi utama(): Bulat {
    konst instansi = IntervalPohon::inisialisasi()
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