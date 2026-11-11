# garuda_blockchain_pow_hashcash

> **Modul:** `BlockchainPowHashcash` • **Kategori:** Blockchain & Web3 • **Versi:** `0.1.0`

Konsensus Proof-of-Work (PoW) berbasis Hashcash dengan penyesuaian tingkat kesulitan dinamis, nonce mining loop, dan target hash.

## Arsitektur & Tujuan
Pencegahan spam jaringan P2P, mekanisme konsensus penambangan terdistribusi, dan timestamping.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_blockchain_pow_hashcash = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_blockchain_pow_hashcash
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BlockchainPowHashcash

publik fungsi utama(): Bulat {
    konst instansi = BlockchainPowHashcash::inisialisasi()
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