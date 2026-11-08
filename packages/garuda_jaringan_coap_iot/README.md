# garuda_jaringan_coap_iot

> **Modul:** `CoapIot` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Constrained Application Protocol RFC 7252 untuk perangkat embedded berdaya rendah.

## Arsitektur & Tujuan
Komponen esensial untuk ekosistem bahasa Garuda pada domain Jaringan & Protokol.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_coap_iot = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_coap_iot
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor CoapIot

publik fungsi utama(): Bulat {
    konst instansi = CoapIot::inisialisasi()
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