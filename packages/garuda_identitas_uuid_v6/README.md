# garuda_identitas_uuid_v6

> **Modul:** `IdentitasUuidV6` • **Kategori:** Utilitas Sistem

Generator UUID versi 4 (acak kriptografis) dan versi 7 (time-ordered).

## Alasan Pengayaan Ekosistem Garuda
Pengidentifikasi unik global untuk entitas database dan transaksi.

## Instalasi via GPM
```bash
gpm pasang garuda_identitas_uuid_v6
```

## Contoh Penggunaan
```garuda
impor IdentitasUuidV6

publik fungsi utama(): Bulat {
    konst cfg = IdentitasUuidV6::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.