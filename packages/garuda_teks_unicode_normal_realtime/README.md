# garuda_teks_unicode_normal_realtime

> **Modul:** `UnicodeNormalRealtime` • **Kategori:** Teks & Pemrosesan Bahasa • **Versi:** `0.1.0`

Varian Realtime berkinerja tinggi: Normalisasi teks Unicode bentuk NFC, NFD, NFKC, dan NFKD sesuai standar.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur realtime untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_teks_unicode_normal_realtime = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_teks_unicode_normal_realtime
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor UnicodeNormalRealtime

publik fungsi utama(): Bulat {
    konst instansi = UnicodeNormalRealtime::inisialisasi()
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