# garuda_sistem_argumen_cli_aman

> **Modul:** `ArgumenCliAman` • **Kategori:** Sistem & OS • **Versi:** `0.1.0`

Varian Aman berkinerja tinggi: Parser opsi baris perintah POSIX dengan bantuan deskripsi otomatis dan validasi tipe.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur aman untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_sistem_argumen_cli_aman = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_sistem_argumen_cli_aman
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor ArgumenCliAman

publik fungsi utama(): Bulat {
    konst instansi = ArgumenCliAman::inisialisasi()
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