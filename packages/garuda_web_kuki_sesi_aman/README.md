# garuda_web_kuki_sesi_aman

> **Modul:** `KukiSesiAman` • **Kategori:** Web & Template • **Versi:** `0.1.0`

Varian Aman berkinerja tinggi: Manajemen cookie HTTP dengan atribut Secure, HttpOnly, SameSite, dan penanganan masa berlaku.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur aman untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_web_kuki_sesi_aman = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_web_kuki_sesi_aman
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KukiSesiAman

publik fungsi utama(): Bulat {
    konst instansi = KukiSesiAman::inisialisasi()
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