# garuda_teks_ansi_pewarna

> **Modul:** `AnsiPewarna` • **Kategori:** Teks & Pemrosesan Bahasa • **Versi:** `0.1.0`

Pemformat gaya teks konsol terminal dengan kode warna ANSI 16 dan 256 warna.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Teks & Pemrosesan Bahasa.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_teks_ansi_pewarna = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_teks_ansi_pewarna
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor AnsiPewarna

publik fungsi utama(): Bulat {
    konst instansi = AnsiPewarna::inisialisasi()
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