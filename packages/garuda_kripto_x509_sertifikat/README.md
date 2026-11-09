# garuda_kripto_x509_sertifikat

> **Modul:** `X509Sertifikat` • **Kategori:** Keamanan & Kriptografi • **Versi:** `0.1.0`

Validasi rantai sertifikat publik X.509 dan penguraian ekstensi CRL/OCSP.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Keamanan & Kriptografi.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kripto_x509_sertifikat = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kripto_x509_sertifikat
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor X509Sertifikat

publik fungsi utama(): Bulat {
    konst instansi = X509Sertifikat::inisialisasi()
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