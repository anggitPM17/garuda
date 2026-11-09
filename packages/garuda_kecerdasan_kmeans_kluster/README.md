# garuda_kecerdasan_kmeans_kluster

> **Modul:** `KmeansKluster` • **Kategori:** Pembelajaran Mesin & AI • **Versi:** `0.1.0`

Algoritma pengelompokan data tanpa pengawasan K-Means dengan inisialisasi ++.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Pembelajaran Mesin & AI.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kecerdasan_kmeans_kluster = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kecerdasan_kmeans_kluster
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KmeansKluster

publik fungsi utama(): Bulat {
    konst instansi = KmeansKluster::inisialisasi()
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