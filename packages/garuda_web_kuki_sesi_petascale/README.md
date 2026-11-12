# garuda_web_kuki_sesi_petascale

> **Modul:** `KukiSesiPetascale` • **Kategori:** Web & Template • **Versi:** `0.1.0`

Varian Petascale berkinerja tinggi: Manajemen cookie HTTP dengan atribut Secure, HttpOnly, SameSite, dan penanganan masa berlaku.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur petascale untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_web_kuki_sesi_petascale = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_web_kuki_sesi_petascale
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KukiSesiPetascale

publik fungsi utama(): Bulat {
    konst instansi = KukiSesiPetascale::inisialisasi()
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