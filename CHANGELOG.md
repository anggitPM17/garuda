# 🦅 Catatan Perubahan (*Changelog*) Garuda Engine

## v67.0.0 (2026-10-24) — Complete Language, Compiler, & GVM Distributed Runtime

### 🚀 Fase 1: Compiler & Bytecode Format
- **Sprint 1.1**: Production Lexer & Tokenizer dwibahasa Nusantara & Elixir dengan pelacakan baris/kolom.
- **Sprint 1.2**: Recursive Descent Parser & Precedence Climbing 11-tingkat dengan pohon AST.
- **Sprint 1.3**: Code Generator AST ke instruksi GVM Opcodes & alokasi register X.
- **Sprint 1.4**: Primitif Aktor (`cakra`, `<-`, `tampa`, `diri`) & Pattern Matching (`wildcard`, `var`, `tuple`).
- **Sprint 1.5**: Serialisasi Biner Bytecode `.gvm` dengan 32-bytes header & FNV1a Checksum.
- **Sprint 1.6**: CLI resmi `grdc` (`compile`, `run`, `repl`, `version`) & 5 program contoh.

### ⚡ Fase 2: High-Performance Runtime
- **Sprint 2.1**: Tail Call Optimization (TCO) $O(1)$ stack usage (10 Juta iterasi dalam 821 ms).
- **Sprint 2.2**: Process Isolation (10.000 aktor paralel dengan state terisolasi penuh).
- **Sprint 2.3**: Preemptive Scheduler berbasis 4000 reductions per slice.
- **Sprint 2.4**: Generational GC per-proses tanpa *global stop-the-world pause* (Jeda < 10 ms).
- **Sprint 2.5**: Million-Scale Concurrency (1.000.000 aktor aktif dalam RAM < 2.5 GB).
- **Sprint 2.6**: Fault Isolation & Kill Actor Memory Reclaim.

### 🌐 Fase 3: Distributed System Infrastructure
- **Sprint 3.1**: Production Raft Consensus Protocol (Leader election & Log replication).
- **Sprint 3.2**: Distributed Process Registry (`diri_global`) ter-sinkronisasi via Raft.
- **Sprint 3.3**: Cross-Platform TCP Socket Transport Layer (Winsock2 & BSD Sockets).
- **Sprint 3.4**: Distributed PubSub Engine (Broadcast pesan masif lintas server VPS).
- **Sprint 3.5**: Hot Code Reloading & Zero Downtime Upgrade (`v1 -> v4`).

### 🛠️ Fase 4: Developer Experience
- **Sprint 4.1**: Dokumentasi Lengkap (`README.md`, `tutorial.md`, `sintaks.md`, `arsitektur.md`, `distributed.md`) & CLI `grdc`.
