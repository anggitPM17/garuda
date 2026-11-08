# garuda_basisdata_kv

> **Modul:** `BasisDataKv` • **Kategori:** Penyimpanan & Basis Data • **Versi:** `0.1.0`

Penyimpanan kunci-nilai persistensi nyata berbasis Write-Ahead Log (WAL) dengan pemulihan pasca-crash dan kompaksi.

## Arsitektur & Tujuan
Persistensi lokal nir-SQL mikrodetik untuk modul backend dan penyimpanan sesi runtime.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_basisdata_kv = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_basisdata_kv
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BasisDataKv

publik fungsi utama(): Bulat {
    konst instansi = BasisDataKv::inisialisasi()
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