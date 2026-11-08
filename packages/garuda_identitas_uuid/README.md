# garuda_identitas_uuid

> **Modul:** `IdentitasUuid` • **Kategori:** Utilitas Sistem • **Versi:** `0.1.0`

Generator dan parser UUID versi 4 (CSPRNG) dan versi 7 (time-ordered timestamp 48-bit) dengan validasi format RFC 4122.

## Arsitektur & Tujuan
Pengidentifikasi unik global berurutan untuk entitas database dan pelacakan transaksi terdistribusi.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_identitas_uuid = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_identitas_uuid
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor IdentitasUuid

publik fungsi utama(): Bulat {
    konst instansi = IdentitasUuid::inisialisasi()
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