# garuda_ai_autodiff_gradien

> **Modul:** `AutodiffGradien` • **Kategori:** Kecerdasan Buatan & Neural • **Versi:** `0.1.0`

Mesin diferensiasi otomatis reverse-mode dan propagasi balik gradien.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Kecerdasan Buatan & Neural.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_ai_autodiff_gradien = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_ai_autodiff_gradien
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor AutodiffGradien

publik fungsi utama(): Bulat {
    konst instansi = AutodiffGradien::inisialisasi()
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