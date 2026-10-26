# 🦅 Panduan & Tutorial Bahasa Pemrograman Garuda

Selamat datang di Panduan Resmi Bahasa Pemrograman Garuda! Tutorial ini terdiri dari 10 bab yang dirancang untuk membimbing Anda dari konsep paling dasar hingga penerapan sistem terdistribusi skala besar.

---

## Bab 1: Hello World

Buat berkas `halo.grd`:

```garuda
modul Halo

fungsi utama() ->
    cetak("Sugeng rawuh, dunia!")
rampung
```

Jalankan via CLI `grdc`:
```bash
$ grdc run halo.grd
🖨️ [GARUDA PRINT] "Sugeng rawuh, dunia!"
✅ Hasil Eksekusi Program: "Sugeng rawuh, dunia!"
```

---

## Bab 2: Variabel & Ekspresi

Di Garuda, variabel diawali dengan huruf kapital untuk *binding*:

```garuda
modul VariabelDemo

fungsi utama() ->
    Akv = 10
    Bkv = 20
    Hasil = (Akv + Bkv) * 2
    cetak(Hasil)
    Hasil
rampung
```

---

## Bab 3: Percabangan Kondisional (`yen/lain`)

```garuda
modul KondisionalDemo

fungsi cek_nilai(N) ->
    yen N >= 75 ->
        :lulus
    lain ->
        :tidak_lulus
    rampung
rampung

fungsi utama() ->
    cek_nilai(80)
rampung
```

---

## Bab 4: Pattern Matching (`tinjau/ing`)

Pencocokan pola (*Pattern Matching*) memungkinkan destrukturisasi data tuple/atom:

```garuda
modul PatternDemo

fungsi proses_respons(TupleData) ->
    tinjau TupleData ing
        {:ok, Hasil} -> Hasil
        {:error, Pesan} -> Pesan
        _ -> :tidak_dikenal
    rampung
rampung

fungsi utama() ->
    proses_respons({:ok, 100})
rampung
```

---

## Bab 5: Fungsi & Rekursi Ekor (TCO)

Garuda secara otomatis mengoptimasi rekursi ekor (*Tail Call Optimization*) sehingga loop 10.000.000 iterasi berjalan dengan memori stack tetap $O(1)$:

```garuda
modul RekursiDemo

fungsi loop(N) ->
    yen N > 0 ->
        loop(N - 1)
    lain ->
        42
    rampung
rampung

fungsi utama() ->
    loop(10000000) # 10 Juta Iterasi tanpa stack overflow!
rampung
```

---

## Bab 6: Aktor Pertama (`cakra`, `<-`, `tampa`)

Proses aktor dibuat dengan `cakra`, dikirimi pesan dengan `<-`, dan menerima pesan dengan `tampa`:

```garuda
modul AktorSederhana

fungsi utama() ->
    Pid = cakra(fungsi() ->
        tampa
            {salam, Pesan} -> cetak(Pesan)
        rampung
    rampung)

    Pid <- {salam, "Halo dari Aktor Induk!"}
rampung
```

---

## Bab 7: Multi-Actor & Ping-Pong

```garuda
modul PingPong

fungsi server() ->
    tampa
        {ping, Dari} -> Dari <- {pong, diri}
    rampung
rampung

fungsi utama() ->
    ServerPid = cakra(fungsi() -> server() rampung)
    ServerPid <- {ping, diri}
    tampa
        {pong, Sender} -> cetak("Pesan PONG diterima!")
    rampung
rampung
```

---

## Bab 8: Supervisor & Fault Tolerance

Proses aktor Garuda terisolasi secara total. Jika satu aktor mati/dikill, aktor lain tetap berjalan normal:

```garuda
modul SupervisorDemo

fungsi worker(Id) ->
    yen Id == 2 ->
        musnah(diri) # Simulasi crash
    lain ->
        :ok
    rampung
rampung

fungsi utama() ->
    P1 = cakra(fungsi() -> worker(1) rampung)
    P2 = cakra(fungsi() -> worker(2) rampung)
    :selesai
rampung
```

---

## Bab 9: Distributed System (Raft + Registry + PubSub)

Aktor dapat dipublikasikan ke seluruh kluster server VPS menggunakan *Global Registry* dan *Distributed PubSub*:

```garuda
modul DistributedDemo

fungsi utama() ->
    # Mendaftarkan nama proses global
    # daftar("kalkulator_1", diri)
    
    # Broadcast pesan ke seluruh node
    # publish("chat_global", "Pesan untuk seluruh kluster!")
    :ok
rampung
```

---

## Bab 10: Hot Code Reloading (Zero Downtime)

Kompilasi dan *upgrade* versi modul biner `.gvm` di memori secara *live* tanpa menghentikan aktor yang sedang berjalan:

```bash
$ grdc compile versi1.grd
$ grdc run versi1.gvm

# Lakukan upgrade live tanpa restart VM:
$ grdc compile versi2.grd
🔥 [GARUDA HOT RELOAD] Live upgraded module 'Aplikasi' v1 -> v2 (Zero Downtime)!
