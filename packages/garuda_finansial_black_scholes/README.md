# garuda_finansial_black_scholes

> **Modul:** `FinansialBlackScholes` • **Kategori:** Finansial & Kuantitatif • **Versi:** `0.1.0`

Penetapan harga opsi Black-Scholes-Merton analitis untuk Call & Put Eropa serta perhitungan parameter sensitivitas The Greeks (Delta, Gamma, Vega, Theta, Rho).

## Arsitektur & Tujuan
Penilaian derivatif keuangan, hedging risiko portofolio pasar modal, dan estimasi volatilitas implisit.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_finansial_black_scholes = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_finansial_black_scholes
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor FinansialBlackScholes

publik fungsi utama(): Bulat {
    konst instansi = FinansialBlackScholes::inisialisasi()
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