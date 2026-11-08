# garuda_basisdata_tsdb_waktu

> **Modul:** `TsdbWaktu` • **Kategori:** Penyimpanan & Basis Data • **Versi:** `0.1.0`

Database deret waktu dengan kompresi Gorilla Delta-of-Delta untuk telemetri.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Penyimpanan & Basis Data.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_basisdata_tsdb_waktu = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_basisdata_tsdb_waktu
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor TsdbWaktu

publik fungsi utama(): Bulat {
    konst instansi = TsdbWaktu::inisialisasi()
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