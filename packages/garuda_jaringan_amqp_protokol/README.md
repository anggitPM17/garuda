# garuda_jaringan_amqp_protokol

> **Modul:** `AmqpProtokol` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Protokol antrean pesan AMQP 0-9-1 untuk pertukaran direktori dan routing key.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Jaringan & Protokol.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_amqp_protokol = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_amqp_protokol
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor AmqpProtokol

publik fungsi utama(): Bulat {
    konst instansi = AmqpProtokol::inisialisasi()
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