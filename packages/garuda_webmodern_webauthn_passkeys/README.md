# garuda_webmodern_webauthn_passkeys

> **Modul:** `WebauthnPasskeys` • **Kategori:** Pengembangan Website Modern • **Versi:** `0.1.0`

Sistem autentikasi nir-kata sandi FIDO2 WebAuthn Passkeys dan enkripsi sesi aman.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Pengembangan Website Modern.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_webmodern_webauthn_passkeys = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_webmodern_webauthn_passkeys
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor WebauthnPasskeys

publik fungsi utama(): Bulat {
    konst instansi = WebauthnPasskeys::inisialisasi()
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