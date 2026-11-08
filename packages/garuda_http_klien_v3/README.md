# garuda_http_klien_v3

> **Modul:** `HttpKlienV3` • **Kategori:** Jaringan & Web

Klien HTTP/1.1 dan HTTP/2 asinkron dengan pooling koneksi otomatis.

## Alasan Pengayaan Ekosistem Garuda
Mendukung integrasi API eksternal dan microservices.

## Instalasi via GPM
```bash
gpm pasang garuda_http_klien_v3
```

## Contoh Penggunaan
```garuda
impor HttpKlienV3

publik fungsi utama(): Bulat {
    konst cfg = HttpKlienV3::inisialisasi()
    cetak("Paket aktif: " + cfg.versi)
    kembalikan 0
}
```

## Lisensi
Lisensi MIT © 2026 Pengembang Bahasa Garuda.