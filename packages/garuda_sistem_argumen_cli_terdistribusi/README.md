# garuda_sistem_argumen_cli_terdistribusi

> **Modul:** `ArgumenCliTerdistribusi` • **Kategori:** Sistem & OS • **Versi:** `0.1.0`

Varian Terdistribusi berkinerja tinggi: Parser opsi baris perintah POSIX dengan bantuan deskripsi otomatis dan validasi tipe.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur terdistribusi untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_sistem_argumen_cli_terdistribusi = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_sistem_argumen_cli_terdistribusi
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor ArgumenCliTerdistribusi

publik fungsi utama(): Bulat {
    konst instansi = ArgumenCliTerdistribusi::inisialisasi()
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