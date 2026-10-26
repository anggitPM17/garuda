#include "../include/garuda/beam/beam.hpp"
#include <cassert>
#include <iostream>

using namespace garuda::beam;
using namespace garuda::elixir;

void test_crypto() {
    std::cout << "[TEST 1/21] Running Crypto & JWT tests..." << std::endl;
    std::string secret = "richat_super_secret_key";
    std::string token = Crypto::generate_jwt("user_12345", secret);
    assert(!token.empty());

    std::string uid;
    bool valid = Crypto::verify_jwt(token, secret, uid);
    assert(valid);
    assert(uid == "user_12345");

    std::string otp = Crypto::generate_otp();
    assert(otp.length() == 6);

    std::string uuid = Crypto::generate_uuid("moment_");
    assert(uuid.rfind("moment_", 0) == 0);

    std::cout << "  ✓ Crypto & JWT tests PASS" << std::endl;
}

void test_mnesia() {
    std::cout << "[TEST 2/21] Running Mnesia Database tests..." << std::endl;
    Mnesia db("./test_garuda_db");
    db.init_richat_schema();

    // Test User Table Write & Read
    db.write("richat_user", Term("user_1001"), Term("{\"name\":\"Anggit\",\"email\":\"anggit@richat.my.id\"}"));
    auto res = db.read("richat_user", Term("user_1001"));
    assert(res.has_value());
    assert(to_string(*res).find("Anggit") != std::string::npos);

    // Test Group Metadata Update
    db.write("richat_group", Term("grp_001"), Term("{\"name\":\"Tim Proyek G4\",\"expires_at\":1795000000}"));
    auto grp = db.read("richat_group", Term("grp_001"));
    assert(grp.has_value());

    std::cout << "  ✓ Mnesia Database tests PASS" << std::endl;
}

void test_gen_server() {
    std::cout << "[TEST 3/21] Running GenServer & OTP tests..." << std::endl;
    Runtime rt;
    EventLoop loop(rt, ClockMode::Virtual);

    struct EchoServer : GenServer {
        CallResult handle_call(Process&, const Term& req, Pid) override {
            return {tuple({atom("reply"), req}), std::nullopt};
        }
        void handle_cast(Process&, const Term&) override {}
    };

    Pid server = start_gen_server(rt, std::make_shared<EchoServer>());
    auto reply = loop.call(server, atom("ping"));
    assert(reply.has_value());

    std::cout << "  ✓ GenServer & OTP tests PASS" << std::endl;
}

void test_cowboy_router() {
    std::cout << "[TEST 4/21] Running Cowboy-Grade Router tests..." << std::endl;
    CowboyRouter router;

    router.get("/api/groups/:id/metadata", [](const HttpRequest&, const std::map<std::string, std::string>& bindings) {
        HttpResponse resp;
        resp.status_code = 200;
        resp.body = "{\"group_id\":\"" + bindings.at("id") + "\",\"status\":\"success\"}";
        return resp;
    });

    HttpRequest req;
    req.method = HttpMethod::Get;
    req.path = "/api/groups/grp_12345/metadata";

    HttpResponse resp = router.dispatch(req);
    assert(resp.status_code == 200);
    assert(resp.body.find("grp_12345") != std::string::npos);

    std::cout << "  ✓ Cowboy Router tests PASS" << std::endl;
}

void test_elixir_abstractions() {
    std::cout << "[TEST 5/21] Running Elixir-Grade Abstraction tests..." << std::endl;

    // 1. Keyword List
    KeywordList kw;
    kw.put("name", Term("Anggit"));
    kw.put("role", Term("owner"));
    assert(kw.has_key("name"));
    assert(to_string(*kw.get("name")) == "\"Anggit\"");

    // 2. Struct
    garuda::elixir::Struct user_struct("User");
    user_struct.set("id", Term("user_001"));
    user_struct.set("name", Term("Anggit"));
    assert(user_struct.struct_name() == "User");
    assert(user_struct.to_string().find("%User{") != std::string::npos);

    // 3. Enum
    std::vector<int> nums = {1, 2, 3, 4, 5};
    auto doubled = garuda::elixir::Enum::map(nums, [](int x) { return x * 2; });
    assert(doubled[0] == 2 && doubled[4] == 10);

    auto evens = garuda::elixir::Enum::filter(nums, [](int x) { return x % 2 == 0; });
    assert(evens.size() == 2 && evens[0] == 2 && evens[1] == 4);

    int sum = garuda::elixir::Enum::reduce(nums, 0, [](int x, int acc) { return acc + x; });
    assert(sum == 15);

    // 4. Pipe Operator (|>)
    auto piped = make_pipe(10)
        .pipe([](int x) { return x * 2; })
        .pipe([](int x) { return x + 5; });
    assert(piped.value() == 25);

    std::cout << "  ✓ Elixir-Grade Abstraction tests PASS" << std::endl;
}

void test_media_storage() {
    std::cout << "[TEST 6/21] Running Media Storage & Dedup tests..." << std::endl;
    MediaStorageEngine media_engine("./test_garuda_media");

    std::string sample_data = "RICHAT_SAMPLE_IMAGE_BINARY_DATA";
    auto meta1 = media_engine.store_file("chat", "sample.jpg", sample_data, "image/jpeg");
    assert(meta1.has_value());

    // Deduplication test: storing same binary content reuses existing hash
    auto meta2 = media_engine.store_file("chat", "sample_copy.jpg", sample_data, "image/jpeg");
    assert(meta2.has_value());
    assert(meta1->hash == meta2->hash);

    std::string read_back;
    bool ok = media_engine.read_file_content(meta1->file_id, read_back);
    assert(ok);
    assert(read_back == sample_data);

    std::cout << "  ✓ Media Storage & Dedup tests PASS" << std::endl;
}

void test_telemetry_and_application() {
    std::cout << "[TEST 7/21] Running Telemetry & OTP Application tests..." << std::endl;

    // 1. Application Env
    Application::set_env("richat", "node_port", Term(8080));
    auto env_val = Application::get_env("richat", "node_port");
    assert(env_val.has_value());
    assert(*env_val->get_if<std::int64_t>() == 8080);

    // 2. Telemetry Exporters
    RuntimeStats stats{};
    stats.spawned = 100;
    stats.messages = 500;
    stats.reductions = 20000;

    std::string prom = TelemetryEngine::export_prometheus(stats, 12);
    assert(prom.find("garuda_processes_active 12") != std::string::npos);

    std::string json_metrics = TelemetryEngine::export_json(stats, 12);
    assert(json_metrics.find("\"engine\": \"Garuda GVM v67.0.0\"") != std::string::npos);

    std::cout << "  ✓ Telemetry & OTP Application tests PASS" << std::endl;
}

void test_ecto_query_builder() {
    std::cout << "[TEST 8/21] Running Elixir Ecto Query Builder tests..." << std::endl;
    Mnesia db("./test_garuda_db");
    db.init_richat_schema();

    db.write("richat_user", Term("u1"), Term("{\"name\":\"Anggit\",\"role\":\"admin\"}"));
    db.write("richat_user", Term("u2"), Term("{\"name\":\"Budi\",\"role\":\"user\"}"));

    auto result = Ecto::from("richat_user")
        .where([](const Term&, const Term& val) {
            return to_string(val).find("Anggit") != std::string::npos;
        })
        .limit(1)
        .one(db);

    assert(result.has_value());
    assert(to_string(*result).find("Anggit") != std::string::npos);

    std::cout << "  ✓ Elixir Ecto Query Builder tests PASS" << std::endl;
}

void test_million_scale_concurrency() {
    std::cout << "[TEST 9/21] Running Million-Scale Concurrency & Lock-Free Queue tests..." << std::endl;

    // 1. Slab Allocator Test
    SlabAllocator<int> slab;
    int* p1 = slab.allocate();
    *p1 = 42;
    assert(*p1 == 42);
    slab.deallocate(p1);

    // 2. Lock-Free MPMC Queue Test
    LockFreeMpmcQueue<int, 1024> queue;
    bool pushed = queue.push(1001);
    assert(pushed);
    auto popped = queue.pop();
    assert(popped.has_value());
    assert(*popped == 1001);

    // 3. Work-Stealing Multi-Core Scheduler Pool
    WorkStealingPool pool(4);
    pool.start();

    std::atomic<int> completed_tasks{0};
    for (int i = 0; i < 100; ++i) {
        pool.submit(i, [&completed_tasks]() {
            completed_tasks.fetch_add(1);
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.stop();

    assert(completed_tasks.load() == 100);

    std::cout << "  ✓ Million-Scale Concurrency & Lock-Free Queue tests PASS" << std::endl;
}

void test_gvm_interpreter_opcodes() {
    std::cout << "[TEST 10/21] Running GVM BEAM Opcode & Register Interpreter tests..." << std::endl;

    std::vector<Instruction> program = {
        {Opcode::OpMove, {Operand{RegType::Literal, 0, Term(100)}, Operand{RegType::X, 0, Term()}}},
        {Opcode::OpMove, {Operand{RegType::Literal, 0, Term(200)}, Operand{RegType::X, 1, Term()}}},
        {Opcode::OpAdd, {Operand{RegType::X, 0, Term()}, Operand{RegType::X, 1, Term()}, Operand{RegType::X, 2, Term()}}},
        {Opcode::OpHalt, {}}
    };

    GvmInterpreter interp(program);
    GvmProcessState vm_state;
    Runtime rt;
    Pid p = rt.spawn([](Process&) { return Status::Done; });

    Process* proc_ptr = rt.find(p);
    assert(proc_ptr != nullptr);

    Status status = interp.execute_slice(*proc_ptr, vm_state);
    assert(status == Status::Done);
    assert(vm_state.x_regs[2].get_if<std::int64_t>() != nullptr);
    assert(*vm_state.x_regs[2].get_if<std::int64_t>() == 300);

    std::cout << "  ✓ GVM BEAM Opcode & Register Interpreter tests PASS" << std::endl;
}

void test_beam_file_reader() {
    std::cout << "[TEST 11/21] Running BEAM Bytecode File Reader tests..." << std::endl;
    std::vector<std::string> atoms;
    std::vector<BeamExportEntry> exports;
    auto program = BeamFileReader::read_and_decode("example.beam", atoms, exports);
    assert(program.has_value());
    assert(atoms.size() >= 3);
    assert(exports.size() >= 2);
    std::cout << "  ✓ BEAM Bytecode File Reader tests PASS" << std::endl;
}

void test_cluster_socket() {
    std::cout << "[TEST 12/21] Running Multi-Node Socket Cluster tests..." << std::endl;
    Runtime rt;
    EventLoop loop(rt, ClockMode::Virtual);
    ClusterSocketNode cluster(loop, "garuda1@127.0.0.1", 9001);
    assert(cluster.start_cluster_listener());
    assert(cluster.connect_peer("garuda2@127.0.0.1", "127.0.0.1", 9002));
    assert(cluster.send_remote_message("garuda2@127.0.0.1", Pid{101}, Term("hello_cluster")));
    std::cout << "  ✓ Multi-Node Socket Cluster tests PASS" << std::endl;
}

void test_generational_gc() {
    std::cout << "[TEST 13/21] Running Per-Process Generational GC tests..." << std::endl;
    GenerationalGC gc(10); // small young heap to trigger GC
    for (int i = 0; i < 50; ++i) {
        gc.allocate(Term(i));
    }
    assert(gc.stats().gc_collections > 0);
    assert(gc.stats().old_heap_words > 0);
    std::cout << "  ✓ Per-Process Generational GC tests PASS" << std::endl;
}

void test_hot_code_reloading() {
    std::cout << "[TEST 14/21] Running Hot Code Reloading & Dynamic Upgrade tests..." << std::endl;
    CodeServer& cs = CodeServer::instance();

    cs.load_module("richat_auth", {{"version", [](Process&) { return Status::Done; }}});
    assert(cs.get_version("richat_auth") == 1);

    // Upgrade live without downtime
    cs.load_module("richat_auth", {{"version", [](Process&) { return Status::Done; }}});
    assert(cs.get_version("richat_auth") == 2);

    assert(cs.purge_old_version("richat_auth"));
    std::cout << "  ✓ Hot Code Reloading & Dynamic Upgrade tests PASS" << std::endl;
}

void test_bitstring_pattern_matching() {
    std::cout << "[TEST 15/21] Running Bitstring Pattern Matcher tests..." << std::endl;
    Bitstring bs;
    bs.append_uint16(256);
    bs.append_binary("hello_binary");

    assert(bs.byte_size() > 3);

    std::uint16_t size = 0;
    std::uint8_t cmd = 0;
    std::string_view payload;
    bool matched = Bitstring::match_header_payload(bs.buffer(), size, cmd, payload);
    assert(matched);
    assert(size == 256);

    std::cout << "  ✓ Bitstring Pattern Matcher tests PASS" << std::endl;
}

void test_priority_scheduler() {
    std::cout << "[TEST 16/21] Running 4-Tier Priority Scheduler tests..." << std::endl;
    PriorityScheduler ps;
    ps.enqueue(Pid{100}, Priority::Low);
    ps.enqueue(Pid{200}, Priority::Max);
    ps.enqueue(Pid{300}, Priority::Normal);

    auto next1 = ps.pop_next();
    assert(next1.has_value());
    assert(next1->id == 200); // Max priority pops first!

    auto next2 = ps.pop_next();
    assert(next2.has_value());
    assert(next2->id == 300); // Normal priority pops next

    std::cout << "  ✓ 4-Tier Priority Scheduler tests PASS" << std::endl;
}

void test_raft_consensus() {
    std::cout << "[TEST 17/21] Running Raft Consensus Protocol tests..." << std::endl;
    RaftNode node1("node_1", {"node_2", "node_3"});
    assert(node1.role() == RaftRole::Follower);

    node1.start_election();
    assert(node1.role() == RaftRole::Leader);

    bool appended = node1.append_entry(node1.term(), Term("state_change"));
    assert(appended);
    assert(node1.log_size() == 1);

    std::cout << "  ✓ Raft Consensus Protocol tests PASS" << std::endl;
}

void test_pubsub_engine() {
    std::cout << "[TEST 18/21] Running Distributed PubSub Engine tests..." << std::endl;
    Runtime rt;
    PubSub pubsub(rt);

    Pid sub1 = rt.spawn([](Process&) { return Status::Wait; });
    Pid sub2 = rt.spawn([](Process&) { return Status::Wait; });

    pubsub.subscribe("room_richat", sub1);
    pubsub.subscribe("room_richat", sub2);

    assert(pubsub.subscriber_count("room_richat") == 2);

    std::size_t delivered = pubsub.publish("room_richat", Term("halo_semua"));
    assert(delivered == 2);

    std::cout << "  ✓ Distributed PubSub Engine tests PASS" << std::endl;
}

void test_circuit_breaker_and_limiter() {
    std::cout << "[TEST 19/21] Running Circuit Breaker & Rate Limiter tests..." << std::endl;

    // 1. Rate Limiter Test
    RateLimiter limiter;
    for (int i = 0; i < 5; ++i) {
        assert(limiter.check_limit("127.0.0.1", 5, 1000));
    }
    assert(!limiter.check_limit("127.0.0.1", 5, 1000));

    // 2. Circuit Breaker Test
    CircuitBreaker cb(2, 500);
    assert(cb.state() == CircuitState::Closed);

    try { cb.call([]() -> int { throw std::runtime_error("fail1"); }); } catch (...) {}
    try { cb.call([]() -> int { throw std::runtime_error("fail2"); }); } catch (...) {}

    assert(cb.state() == CircuitState::Open);

    std::cout << "  ✓ Circuit Breaker & Rate Limiter tests PASS" << std::endl;
}

void test_vps_production_sys_infrastructure() {
    std::cout << "[TEST 20/21] Running Production System & Linux VPS Infrastructure tests..." << std::endl;

    // 1. Logger
    Logger::instance().set_log_file("./test_garuda.log");
    GARUDA_LOG_INFO("SYS_TEST", "Garuda Engine Production System Infrastructure initialized.");

    // 2. Resource limits
    bool opt_ok = ResourceLimits::optimize_for_vps(1024);
    assert(opt_ok);

    // 3. Signal Handler & Systemd Daemon
    SignalHandler::setup_signals([](int sig) {
        std::cout << "Signal callback triggered for sig: " << sig << std::endl;
    });
    Daemon::notify_ready();

    std::cout << "  ✓ Production System & Linux VPS Infrastructure tests PASS" << std::endl;
}

void test_garuda_source_compiler() {
    std::cout << "[TEST 21/21] Running Garuda Language Source Compiler (.grd) tests..." << std::endl;

    std::string source = R"(
        defmodule RichatCore do
            def calculate_total do
                100 + 200
            end
        end
    )";

    auto compiled_mod = garuda::compiler::Compiler::compile_source(source, "RichatCore.grd");
    assert(compiled_mod.has_value());
    assert(compiled_mod->module_name == "RichatCore");
    assert(compiled_mod->instructions.size() >= 3);

    bool written = garuda::compiler::Compiler::write_gvm_bytecode(*compiled_mod, "./RichatCore.gvm");
    assert(written);

    // Execute compiled bytecode directly in GVM Interpreter!
    GvmInterpreter interp(compiled_mod->instructions);
    GvmProcessState vm_state;
    Runtime rt;
    Pid p = rt.spawn([](Process&) { return Status::Done; });

    Process* proc_ptr = rt.find(p);
    assert(proc_ptr != nullptr);

    Status status = interp.execute_slice(*proc_ptr, vm_state);
    assert(status == Status::Done);
    assert(vm_state.x_regs[2].get_if<std::int64_t>() != nullptr);
    assert(*vm_state.x_regs[2].get_if<std::int64_t>() == 300);

    std::cout << "  ✓ Garuda Language Source Compiler (.grd) tests PASS" << std::endl;
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA ENGINE v67.0.0 — FULL EXPANDED 21/21 TEST SUITE\n";
    std::cout << "=======================================================\n\n";

    test_crypto();
    test_mnesia();
    test_gen_server();
    test_cowboy_router();
    test_elixir_abstractions();
    test_media_storage();
    test_telemetry_and_application();
    test_ecto_query_builder();
    test_million_scale_concurrency();
    test_gvm_interpreter_opcodes();
    test_beam_file_reader();
    test_cluster_socket();
    test_generational_gc();
    test_hot_code_reloading();
    test_bitstring_pattern_matching();
    test_priority_scheduler();
    test_raft_consensus();
    test_pubsub_engine();
    test_circuit_breaker_and_limiter();
    test_vps_production_sys_infrastructure();
    test_garuda_source_compiler();

    std::cout << "\n🎉 ALL 21/21 GARUDA EXPANDED MODULE TESTS PASSED PERFECTLY!\n";
    return 0;
}
