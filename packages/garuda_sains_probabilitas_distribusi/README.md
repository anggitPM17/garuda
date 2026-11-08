# garuda_sains_probabilitas_distribusi

> **Modul:** `ProbabilitasDistribusi` • **Kategori:** Statistik & Sains • **Versi:** `0.1.0`

Fungsi kepadatan probabilitas distribusi Normal, Poisson, dan Binomial.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Statistik & Sains.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_sains_probabilitas_distribusi = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_sains_probabilitas_distribusi
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor ProbabilitasDistribusi

publik fungsi utama(): Bulat {
    konst instansi = ProbabilitasDistribusi::inisialisasi()
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