# garuda_sains_statistik_deskriptif_submikro

> **Modul:** `StatistikDeskriptifSubmikro` • **Kategori:** Statistik & Sains • **Versi:** `0.1.0`

Varian Submikro berkinerja tinggi: Penghitungan mean, median, modus, varians, deviasi standar, dan persentil.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur submikro untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_sains_statistik_deskriptif_submikro = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_sains_statistik_deskriptif_submikro
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor StatistikDeskriptifSubmikro

publik fungsi utama(): Bulat {
    konst instansi = StatistikDeskriptifSubmikro::inisialisasi()
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