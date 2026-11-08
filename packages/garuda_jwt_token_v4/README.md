# garuda_jwt_token_v4

> **Modul:** `JwtTokenV4` • **Kategori:** Keamanan & Autentikasi

Pembuat dan pemverifikasi JSON Web Token (JWT) standar HS256/RS256.

## Alasan Pengayaan Ekosistem Garuda
Standar autentikasi token nir-keadaan untuk API Garuda.

## Instalasi via GPM
```bash
gpm pasang garuda_jwt_token_v4
```

## Contoh Penggunaan
```garuda
impor JwtTokenV4

publik fungsi utama(): Bulat {
    konst cfg = JwtTokenV4::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.