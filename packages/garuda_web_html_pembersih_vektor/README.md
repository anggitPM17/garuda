# garuda_web_html_pembersih_vektor

> **Modul:** `HtmlPembersihVektor` • **Kategori:** Web & Template • **Versi:** `0.1.0`

Varian Vektor berkinerja tinggi: Sanitasi markup HTML untuk pencegahan celah keamanan Cross-Site Scripting (XSS).

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur vektor untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_web_html_pembersih_vektor = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_web_html_pembersih_vektor
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor HtmlPembersihVektor

publik fungsi utama(): Bulat {
    konst instansi = HtmlPembersihVektor::inisialisasi()
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