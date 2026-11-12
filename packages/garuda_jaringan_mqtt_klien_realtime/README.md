# garuda_jaringan_mqtt_klien_realtime

> **Modul:** `MqttKlienRealtime` • **Kategori:** Jaringan & Protokol • **Versi:** `0.1.0`

Varian Realtime berkinerja tinggi: Protokol pesan IoT MQTT 3.1.1 dan 5.0 dengan QoS level 0, 1, dan 2.

## Arsitektur & Tujuan
Optimalisasi khusus arsitektur realtime untuk beban komputasi terisolasi di GVM.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_jaringan_mqtt_klien_realtime = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_jaringan_mqtt_klien_realtime
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor MqttKlienRealtime

publik fungsi utama(): Bulat {
    konst instansi = MqttKlienRealtime::inisialisasi()
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