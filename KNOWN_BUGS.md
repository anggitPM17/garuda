# Known Bugs — Garuda v67.0.0

## KB-001: `List.peta` / `List.saring` + Lambda Anonim → HANG

**Status**: Open (deferred)  
**Ditemukan**: Sesi debugging W9/W13d  
**Severity**: High (blocker untuk higher-order function dengan lambda via stdlib)  

### Reproduksi

```garuda
modul TestW9
impor "std/json.grd"
fungsi utama() ->
    List.panjang(List.peta([1, 2, 3], fungsi(X) -> X * 2 rampung))
rampung
```

**Hasil**: HANG (infinite loop di GVM).  
**Expected**: `3`.  

### Gejala

- Test W9 (`List.peta` + lambda 1-param) dan W13d (`peta(saring(...))`) HANG.
- W10 (`List.lipat` + lambda 2-param) PASS — perbedaan: `lipat` memanggil `F(Acc, H)` di tail call, sedangkan `peta` memanggil `F(H)` di dalam argumen `tambah(Acc, F(H))`.
- W13a/b/c (lambda via variabel langsung, bukan via `peta`/`saring`) PASS.

### Akar Masalah (Hipotesis)

1. `protect_param_regs(2)` memindahkan `F` dari `X[1]` ke `X[16]` saat mengkompilasi `tambah(Acc, F(H))`.
2. `CallExpr F(H)` meng-emit `OpCall X[16]` (atau `X[18]`).
3. `gen_tail_call` hanya mengisi `X[0..arity-1]` (parameter kanonis), **tidak mengisi `X[16]`**.
4. Iterasi ke-2: `X[16] = 0` (undefined) → `OpCall X[16]` → IP 0 → infinite loop.

### Percobaan Perbaikan yang Gagal

| # | Pendekatan | Hasil |
|---|---|---|
| 1 | Save/restore `var_map` di `gen_tail_call` | HANG |
| 2 | Emit `OpMove` tambahan di `gen_tail_call` | HANG |
| 3 | Pre-colored murni (tidak pindahkan `F`) | HANG (F tertimpa argumen) |
| 4 | Kombinasi (protect asli + `var_map` langsung + reload `X[16]`) | HANG (reload tidak mencakup `X[16]`) |
| 5 | Workaround `peta_loop` statement terpisah | HANG hilang, tapi parser `tinjau` hanya mendukung 1 ekspresi per klausa |

### Workaround Sementara

**Workaround yang terbukti menghilangkan HANG** (tapi membutuhkan dukungan multi-statement per klausa di parser `tinjau`):

```garuda
fungsi peta_loop(Daftar, F, Acc) ->
    tinjau Daftar ing
        [] -> Acc
        [H | T] ->
            Hasil = F(H)
            Acc2 = tambah(Acc, Hasil)
            peta_loop(T, F, Acc2)
        _ -> Acc
    rampung
rampung
```

### Rekomendasi Perbaikan (untuk Masa Depan)

1. **Loop reload terpisah di `gen_tail_call`** — iterasi `param_names`, bukan `temp_regs.size()`:
   ```cpp
   for (std::size_t i = 0; i < current_func_->param_names.size(); ++i) {
       const std::string& pname = current_func_->param_names[i];
       auto it = current_func_->var_map.find(pname);
       if (it != current_func_->var_map.end() && it->second != i) {
           emit(Opcode::OpMove, {
               Operand{RegType::X, static_cast<std::uint16_t>(i), Term()},
               Operand{RegType::X, it->second, Term()}
           });
       }
   }
   ```
2. Cek `handle_builtin` untuk `List.tambah` dan `List.panjang`.
3. Pertimbangkan **regalloc pass** terpisah (seperti LLVM) alih-alih ad-hoc `var_map`.

### File Terkait

- `include/garuda/compiler/codegen.hpp` — `protect_param_regs`, `gen_tail_call`, `CallExpr`
- `std/list.grd` — `peta_loop`, `saring_loop`, `tambah`
- `tests/test_isolate_binop.cpp` — W9, W13d
- `include/garuda/beam/tools/script_engine.hpp` — `handle_builtin` untuk `List.tambah`, `List.panjang`

### Referensi

- [Crafting Interpreters — Environment](https://craftinginterpreters.com/resolving-and-binding.html)
- [Cornell CS3410 — Calling Convention](https://www.cs.cornell.edu/courses/cs3410/2019sp/schedule/slides/10-calling-notes-bw.pdf)
- [LLVM — Register Allocation](https://llvm.org/docs/CodeGenerator.html#register-allocation)
