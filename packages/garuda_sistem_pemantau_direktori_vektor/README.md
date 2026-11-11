# garuda_sistem_pemantau_direktori_vektor

> **Modul:** `PemantauDirektoriVektor` • **Kategori:** Sistem & OS • **Versi:** `0.1.0`

Varian Vektor berkinerja tinggi: Pendeteksi perubahan berkas dan direktori sistem berkas secara asinkron.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur vektor untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_sistem_pemantau_direktori_vektor = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_sistem_pemantau_direktori_vektor
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor PemantauDirektoriVektor

publik fungsi utama(): Bulat {
    konst instansi = PemantauDirektoriVektor::inisialisasi()
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