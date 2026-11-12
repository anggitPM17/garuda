# garuda_kripto_shamir_rahasia_realtime

> **Modul:** `ShamirRahasiaRealtime` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

Varian Realtime berkinerja tinggi: Pembagian rahasia ambang batas skema polinomial Lagrange (k dari n).

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur realtime untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto_shamir_rahasia_realtime = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto_shamir_rahasia_realtime
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor ShamirRahasiaRealtime

publik fungsi utama(): Bulat {
    konst instansi = ShamirRahasiaRealtime::inisialisasi()
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