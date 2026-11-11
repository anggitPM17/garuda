# garuda_finansial_risk_analysis

> **Modul:** `FinansialRiskAnalysis` • **Kategori:** Finansial & Kuantitatif • **Versi:** `0.1.0`

Analisis risiko keuangan portofolio kuantitatif: Value at Risk (VaR Parametrik & Monte Carlo), Conditional VaR (Expected Shortfall), dan Sharpe/Sortino Ratio.

## Arsitektur & Tujuan
Kepatuhan manajemen risiko Basel III, stresstest skenario pasar ekstrem, dan optimasi bobot alokasi aset.

## Pemasangan via GPM
Tambahkan ke berkas `grdmix.toml`:
```toml
[dependensi]
garuda_finansial_risk_analysis = "0.1.0"
```

Atau instal langsung melalui CLI:
```bash
gpm pasang garuda_finansial_risk_analysis
```

## Contoh Penggunaan
```garuda
modul AplikasiSaya

impor FinansialRiskAnalysis

publik fungsi utama(): Bulat {
    konst instansi = FinansialRiskAnalysis::inisialisasi()
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