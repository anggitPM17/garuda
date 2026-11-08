# garuda_identitas_uuid_v7

> **Modul:** `IdentitasUuidV7` • **Kategori:** Utilitas Sistem

Generator UUID versi 4 (acak kriptografis) dan versi 7 (time-ordered).

## Alasan Pengayaan Ekosistem Garuda
Pengidentifikasi unik global untuk entitas database dan transaksi.

## Instalasi via GPM
```bash
gpm pasang garuda_identitas_uuid_v7
```

## Contoh Penggunaan
```garuda
impor IdentitasUuidV7

publik fungsi utama(): Bulat {
    konst cfg = IdentitasUuidV7::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.