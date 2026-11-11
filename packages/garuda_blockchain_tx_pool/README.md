# garuda_blockchain_tx_pool

> **Modul:** `BlockchainTxPool` • **Kategori:** Blockchain & Web3 • **Versi:** `0.1.0`

Transaction Memory Pool (Mempool) terurut biaya gas dengan verifikasi nonce urut, deteksi double-spending, dan pengusiran transaksi usang.

## Arsitektur & Tujuan
Pengelola antrean transaksi terdesentralisasi sebelum dikemas ke dalam blok berikutnya.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_blockchain_tx_pool = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_blockchain_tx_pool
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor BlockchainTxPool

publik fungsi utama(): Bulat {
    konst instansi = BlockchainTxPool::inisialisasi()
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