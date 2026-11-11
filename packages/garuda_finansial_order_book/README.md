# garuda_finansial_order_book

> **Modul:** `FinansialOrderBook` • **Kategori:** Finansial & Kuantitatif • **Versi:** `0.1.0`

Mesin pencocokan pesanan (Matching Engine) batas harga waktu nyata (Price-Time Priority FIFO) L2/L3 untuk pasar spot dan derivatif.

## Arsitektur & Tujuan
Inti bursa efek perdagangan frekuensi tinggi (HFT) dengan latensi eksekusi sub-mikrodetik.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_finansial_order_book = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_finansial_order_book
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor FinansialOrderBook

publik fungsi utama(): Bulat {
    konst instansi = FinansialOrderBook::inisialisasi()
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