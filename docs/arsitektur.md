# Arsitektur Internal Garuda Virtual Machine (GVM)

Dokumentasi rancangan sistem internal compiler dan runtime GVM v67.0.0.

---

## 1. Komponen Utama Arsitektur

```
                   +------------------------------+
                   |  Berkas Skrip Garuda (.grd)  |
                   +--------------+---------------+
                                  |
                                  v
                   +------------------------------+
                   |      Lexer & Tokenizer       |
                   +--------------+---------------+
                                  | Token Stream
                                  v
                   +------------------------------+
                   |       Parser & AST           |
                   +--------------+---------------+
                                  | AST Tree
                                  v
                   +------------------------------+
                   | CodeGenerator (codegen.hpp)  |
                   +--------------+---------------+
                                  | Bytecode & Function Table
                                  v
                   +------------------------------+
                   | ScriptEngine (interpreter)   |
                   | - GVM Register Machine       |
                   | - BIF Handler (C++ Native)   |
                   +------------------------------+
```

1. **Lexer & Parser**: Mengubah teks skrip `.grd` menjadi *Abstract Syntax Tree* (AST).
2. **CodeGenerator (`include/garuda/compiler/codegen.hpp`)**: Mengubah AST menjadi instruksi *Register-based Bytecode* dan membangun `function_table`.
3. **ScriptEngine (`include/garuda/beam/tools/script_engine.hpp`)**:
   - Runtime *Interpreter* berbasis register `x_regs` (256 register).
   - Penanganan Built-In Functions (BIF) C++ native (`handle_builtin`).
   - Eksekusi instruksi bytecode `OpCall`, `OpTailCall`, `OpReturn`, `OpSpawn`, `OpSend`, `OpReceive`.

---

## 2. Instruksi Bytecode Utama (Opcodes)

| Opcode | Deskripsi Operasi |
| :--- | :--- |
| `OpMove` | Memindahkan nilai dari register / literal ke register tujuan |
| `OpAdd` / `OpSub` / `OpMul` / `OpDiv` | Operasi aritmatika pada register / literal |
| `OpConcat` | Penggabungan String atau List (`++`) |
| `OpCall` | Pemanggilan fungsi (push `call_stack` dan `frame_stack`, lompat ke IP) |
| `OpTailCall` | Pemanggilan ekor (*Tail Call Optimization*) — mengganti frame tanpa alokasi stack baru |
| `OpReturn` | Pengembalian nilai dari fungsi dan pemulihan frame stack |
| `OpMakeTuple` / `OpGetTupleElem` | Alokasi dan pembacaan elemen Tuple |
| `OpMakeList` / `OpGetListLen` / `OpGetListElem` | Alokasi dan manipulasi List |
| `OpSpawn` / `OpSend` / `OpReceive` | Operasi Actor concurrency GVM |

---

## 3. Penanganan Pemanggilan Ekor BIF (*Tail Call Optimization*)

Saat fungsi skrip Garuda melakukan pemanggilan ekor ke fungsi BIF (misal `fungsi get(P, H) -> HttpServer.daftar_route("GET", P, H) rampung`):
1. `CodeGenerator` mentranspilasi pemanggilan tersebut menjadi `OpTailCall`.
2. Pada GVM interpreter, `OpTailCall` mendeteksi bahwa fungsi target merupakan BIF C++ (`handle_builtin`).
3. Setelah BIF dieksekusi, GVM secara otomatis melakukan pembersihan stack frame (*pop call stack*) dan mengembalikan nilai ke pemanggil fungsi tingkat atas, mencegah kebocoran memori atau *infinite loop*.
