# garuda_grafika_warna_ruang

> **Modul:** `WarnaRuang` • **Kategori:** Matematika & Grafika • **Versi:** `0.1.0`

Konversi ruang warna RGB, HSL, HSV, CIE-Lab, dan perhitungan kontras WCAG.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Matematika & Grafika.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_grafika_warna_ruang = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_grafika_warna_ruang
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor WarnaRuang

publik fungsi utama(): Bulat {
    konst instansi = WarnaRuang::inisialisasi()
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