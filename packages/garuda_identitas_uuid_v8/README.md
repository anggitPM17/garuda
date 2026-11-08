# garuda_identitas_uuid_v8

> **Modul:** `IdentitasUuidV8` • **Kategori:** Utilitas Sistem

Generator UUID versi 4 (acak kriptografis) dan versi 7 (time-ordered).

## Alasan Pengayaan Ekosistem Garuda
Pengidentifikasi unik global untuk entitas database dan transaksi.

## Instalasi via GPM
```bash
gpm pasang garuda_identitas_uuid_v8
```

## Contoh Penggunaan
```garuda
impor IdentitasUuidV8

publik fungsi utama(): Bulat {
    konst cfg = IdentitasUuidV8::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.