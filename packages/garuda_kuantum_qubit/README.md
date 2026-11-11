# garuda_kuantum_qubit

> **Modul:** `KuantumQubit` • **Kategori:** Komputasi Kuantum • **Versi:** `0.1.0`

Simulasi Qubit keadaan superposisi murni dan matriks densitas tereduksi dengan representasi vektor keadaan ruang Hilbert kompleks.

## Arsitektur & Tujuan
Fondasi pemodelan mekanika kuantum dan register komputasi kuantum tingkat dasar.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_kuantum_qubit = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_kuantum_qubit
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor KuantumQubit

publik fungsi utama(): Bulat {
    konst instansi = KuantumQubit::inisialisasi()
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