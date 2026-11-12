# garuda_format_yaml_konfig_submikro

> **Modul:** `YamlKonfigSubmikro` • **Kategori:** Parsing & Format Data • **Versi:** `0.1.0`

Varian Submikro berkinerja tinggi: Parser subset format YAML untuk pembacaan konfigurasi aplikasi deklaratif.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur submikro untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_format_yaml_konfig_submikro = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_format_yaml_konfig_submikro
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor YamlKonfigSubmikro

publik fungsi utama(): Bulat {
    konst instansi = YamlKonfigSubmikro::inisialisasi()
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