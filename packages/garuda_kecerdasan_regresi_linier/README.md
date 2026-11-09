# garuda_kecerdasan_regresi_linier

> **Modul:** `RegresiLinier` • **Kategori:** Pembelajaran Mesin & AI • **Versi:** `0.1.0`

Estimasi garis regresi kuadrat terkecil ordinary least squares.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Pembelajaran Mesin & AI.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kecerdasan_regresi_linier = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kecerdasan_regresi_linier
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor RegresiLinier

publik fungsi utama(): Bulat {
    konst instansi = RegresiLinier::inisialisasi()
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