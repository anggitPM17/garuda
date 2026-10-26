#include "../include/garuda/beam/net/socket_layer.hpp"
#include "../include/garuda/beam/net/raft.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA DISTRIBUTED — SPRINT 3.3 TCP SOCKET CLUSTER TEST SUITE (4 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: 2 Node TCP Handshake & Messaging ===
    std::cout << "[TEST 1/4] Running 2 Node TCP Handshake & Messaging test...\n";
    SocketLayer sock_a("A", 9001);
    SocketLayer sock_b("B", 9002);

    std::string b_recv;
    sock_b.set_message_callback([&](const std::string& from, const std::string& msg) {
        b_recv = from + ":" + msg;
    });

    bool start_a = sock_a.start();
    bool start_b = sock_b.start();
    assert(start_a && start_b);

    bool conn_ok = sock_b.connect_to_peer("A", "127.0.0.1", 9001);
    assert(conn_ok);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    sock_a.send("B", "hello_from_A");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    assert(b_recv == "A:hello_from_A");
    sock_a.stop();
    sock_b.stop();
    std::cout << "  ✓ Test 1: 2 Node TCP Handshake & Messaging PASS\n";

    // === Test 2: 3 Node Broadcast ===
    std::cout << "[TEST 2/4] Running 3 Node TCP Broadcast test...\n";
    SocketLayer s1("N1", 9101);
    SocketLayer s2("N2", 9102);
    SocketLayer s3("N3", 9103);

    std::string s2_msg, s3_msg;
    s2.set_message_callback([&](const std::string& from, const std::string& msg) {
        s2_msg = from + ":" + msg;
    });
    s3.set_message_callback([&](const std::string& from, const std::string& msg) {
        s3_msg = from + ":" + msg;
    });

    s1.start(); s2.start(); s3.start();
    s2.connect_to_peer("N1", "127.0.0.1", 9101);
    s3.connect_to_peer("N1", "127.0.0.1", 9101);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    s1.send("N2", "broadcast_m2");
    s1.send("N3", "broadcast_m3");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    assert(s2_msg == "N1:broadcast_m2");
    assert(s3_msg == "N1:broadcast_m3");

    s1.stop(); s2.stop(); s3.stop();
    std::cout << "  ✓ Test 2: 3 Node TCP Broadcast PASS\n";

    // === Test 3: Raft Consensus over Real TCP Sockets ===
    std::cout << "[TEST 3/4] Running Raft Consensus over Real TCP Sockets test...\n";
    SocketLayer r_s1("R1", 9201);
    SocketLayer r_s2("R2", 9202);
    SocketLayer r_s3("R3", 9203);

    RaftNode raft1("R1", {"R2", "R3"});
    RaftNode raft2("R2", {"R1", "R3"});
    RaftNode raft3("R3", {"R1", "R2"});

    r_s1.set_message_callback([&](const std::string&, const std::string& msg) { raft1.process_network_message(msg); });
    r_s2.set_message_callback([&](const std::string&, const std::string& msg) { raft2.process_network_message(msg); });
    r_s3.set_message_callback([&](const std::string&, const std::string& msg) { raft3.process_network_message(msg); });

    raft1.set_send_callback([&](const std::string& target, const std::string& msg) { r_s1.send(target, msg); });
    raft2.set_send_callback([&](const std::string& target, const std::string& msg) { r_s2.send(target, msg); });
    raft3.set_send_callback([&](const std::string& target, const std::string& msg) { r_s3.send(target, msg); });

    r_s1.start(); r_s2.start(); r_s3.start();
    r_s2.connect_to_peer("R1", "127.0.0.1", 9201);
    r_s3.connect_to_peer("R1", "127.0.0.1", 9201);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    raft1.start_election();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    assert(raft1.role() == RaftRole::Leader);
    assert(raft1.current_term() == 1);
    std::cout << "  ✓ Test 3: Raft Consensus over Real TCP Sockets PASS\n";

    // === Test 4: Chaos Test (Kill Leader Socket, Re-elect over TCP) ===
    std::cout << "[TEST 4/4] Running Chaos Test (Kill Leader Socket) test...\n";
    r_s1.stop(); // Stop Node 1 Socket completely!

    raft2.start_election();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    assert(raft2.role() == RaftRole::Leader);
    assert(raft2.current_term() == 2);

    r_s2.stop(); r_s3.stop();
    std::cout << "  ✓ Test 4: Chaos Test (Kill Leader Socket, Re-elect over TCP) PASS\n";

    std::cout << "\n🎉 ALL 4 TCP SOCKET CLUSTER TESTS PASSED PERFECTLY!\n";
    return 0;
}
