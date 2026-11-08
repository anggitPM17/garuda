# garuda_jwt_token

> **Modul:** `JwtToken` • **Kategori:** Keamanan & Autentikasi • **Versi:** `0.1.0`

Pembuat dan pemverifikasi RFC 7519 JSON Web Token (JWT) standar HS256, base64url tanpa padding, serta validasi exp/nbf/iat.

## Arsitektur & Tujuan
Standar otentikasi token nir-keadaan (stateless) untuk microservices dan API eksternal.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jwt_token = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jwt_token
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor JwtToken

publik fungsi utama(): Bulat {
    konst instansi = JwtToken::inisialisasi()
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