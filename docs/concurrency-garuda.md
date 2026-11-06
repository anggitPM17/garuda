# ⚡ GARUDA ACTOR MODEL & CONCURRENCY GUIDE (GVM ENGINE)

Dokumen ini menjelaskan arsitektur **Concurrency Model berbasis Aktor Lightweight & Erlang-style Fault Tolerance** pada VM Garuda (GVM Engine).

---

## 💡 KONSEP UTAMA

1. **Aktor Lightweight (bukan OS Thread)**: Setiap aktor berjalan terisolasi dengan registri dan mailbox sendiri.
2. **Mailbox FIFO**: Pesan antar aktor dikirim secara asinkron dan diproses sesuai urutan kedatangan (First-In, First-Out).
3. **Supervisor Tree**: Mengawasi aktor pekerja. Jika terjadi crash, supervisor dapat me-restart aktor tersebut (*Let it crash* philosophy).
4. **Cluster Distribution**: Aktor dapat di-spawn di node remote secara transparan.

---

## 💻 1. AKTOR & MESSAGE PASSING (`Process`)

```garuda
modul DemoAktor

fungsi pekerja(ParentPid) ->
    Pesan = Process.receive(2000)
    Process.send(ParentPid, "BALASAN_" ++ Pesan)
rampung

fungsi utama() ->
    Me = Process.self()
    Pid = Process.spawn("pekerja", Me)
    Process.send(Pid, "PING")
    Balasan = Process.receive(2000)
    cetak("Hasil: #{Balasan}")
rampung
```

---

## 🛡️ 2. SUPERVISOR TREE (`Supervisor`)

```garuda
modul DemoSupervisor

fungsi pekerja() ->
    Process.receive(1000)
rampung

fungsi utama() ->
    SupPid = Supervisor.mulai("pekerja", "satu_untuk_satu")
    NewWorkerPid = Supervisor.restart(SupPid)
    Kesehatan = Supervisor.health(SupPid)
    cetak("Restarts: #{Map.ambil(Kesehatan, "restarts")}")
rampung
```

---

## 🌐 3. ASYNC HTTP SERVER NON-BLOCKING (`Server.mulai_async`)

```garuda
modul DemoAsyncWeb

fungsi handle_request(Req) ->
    Path = Map.ambil(Req, "path")
    Http.buat_respons(200, %{"Content-Type" => "text/plain"}, "Async Response for #{Path}")
rampung

fungsi utama() ->
    Server.mulai_async(9090, handle_request, 10000)
rampung
```

---

## 🌐 4. DISTRIBUTED CLUSTER (`Node`)

```garuda
modul DemoCluster

fungsi pekerja_remote() ->
    Process.receive(1000)
rampung

fungsi utama() ->
    Node.mulai("node_utama", "127.0.0.1", 9091)
    Node.hubung("127.0.0.1", 9091)
    RemotePid = Node.remote_spawn("node_utama", "pekerja_remote")
    Node.remote_send(RemotePid, "PESAN_KLASTER")
rampung
```
