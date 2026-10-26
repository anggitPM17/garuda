#include "../include/garuda/beam/net/distributed_pubsub.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA DISTRIBUTED — SPRINT 3.4 DISTRIBUTED PUBSUB TEST SUITE (5 TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: Subscribe & Publish Local ===
    std::cout << "[TEST 1/5] Running Subscribe & Publish Local test...\n";
    RaftNode raft_a("A", {});
    SocketLayer socket_a("A", 9501);
    socket_a.start();

    DistributedPubSub ps_a(raft_a, socket_a);
    std::vector<std::string> delivered_1;

    ps_a.set_delivery_callback([&](Pid pid, const std::string& topic, const std::string& msg) {
        delivered_1.push_back(std::to_string(pid.id) + ":" + topic + ":" + msg);
    });

    ps_a.subscribe("chat", Pid{1});
    ps_a.subscribe("chat", Pid{2});
    ps_a.subscribe("chat", Pid{3});

    assert(ps_a.subscriber_count("chat") == 3);

    ps_a.publish("chat", "hello");
    assert(delivered_1.size() == 3);
    assert(delivered_1[0] == "1:chat:hello");
    assert(delivered_1[1] == "2:chat:hello");
    assert(delivered_1[2] == "3:chat:hello");

    socket_a.stop();
    std::cout << "  ✓ Test 1: Subscribe & Publish Local PASS\n";

    // === Test 2: Unsubscribe ===
    std::cout << "[TEST 2/5] Running Unsubscribe test...\n";
    SocketLayer socket_b("B", 9502);
    socket_b.start();
    DistributedPubSub ps_b(raft_a, socket_b);

    std::vector<std::string> delivered_2;
    ps_b.set_delivery_callback([&](Pid pid, const std::string&, const std::string& msg) {
        delivered_2.push_back(std::to_string(pid.id) + ":" + msg);
    });

    ps_b.subscribe("news", Pid{10});
    ps_b.subscribe("news", Pid{20});
    assert(ps_b.subscriber_count("news") == 2);

    ps_b.unsubscribe("news", Pid{10});
    assert(ps_b.subscriber_count("news") == 1);

    ps_b.publish("news", "headline");
    assert(delivered_2.size() == 1);
    assert(delivered_2[0] == "20:headline");

    socket_b.stop();
    std::cout << "  ✓ Test 2: Unsubscribe PASS\n";

    // === Test 3: 2 Node Cross-Node Publish (KUNCI!) ===
    std::cout << "[TEST 3/5] Running 2 Node Cross-Node Publish test...\n";
    SocketLayer sock_n1("N1", 9601);
    SocketLayer sock_n2("N2", 9602);

    RaftNode r_n1("N1", {"N2"});
    RaftNode r_n2("N2", {"N1"});

    sock_n1.start();
    sock_n2.start();

    sock_n2.connect_to_peer("N1", "127.0.0.1", 9601);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    DistributedPubSub ps_n1(r_n1, sock_n1);
    DistributedPubSub ps_n2(r_n2, sock_n2);

    std::vector<std::string> n1_delivered;
    ps_n1.set_delivery_callback([&](Pid pid, const std::string&, const std::string& msg) {
        n1_delivered.push_back(std::to_string(pid.id) + ":" + msg);
    });

    ps_n1.subscribe("global_chat", Pid{100});
    ps_n1.subscribe("global_chat", Pid{200});

    // Node 2 publishes over socket to Node 1!
    sock_n2.send("N1", "PUBLISH global_chat hello_from_N2");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    assert(n1_delivered.size() == 2);
    assert(n1_delivered[0] == "100:hello_from_N2");
    assert(n1_delivered[1] == "200:hello_from_N2");

    sock_n1.stop();
    sock_n2.stop();
    std::cout << "  ✓ Test 3: 2 Node Cross-Node Publish PASS\n";

    // === Test 4: 3 Node Broadcast ===
    std::cout << "[TEST 4/5] Running 3 Node Broadcast test...\n";
    SocketLayer s1("S1", 9701);
    SocketLayer s2("S2", 9702);
    SocketLayer s3("S3", 9703);

    RaftNode r1("S1", {"S2", "S3"});
    RaftNode r2("S2", {"S1", "S3"});
    RaftNode r3("S3", {"S1", "S2"});

    s1.start(); s2.start(); s3.start();
    s2.connect_to_peer("S1", "127.0.0.1", 9701);
    s3.connect_to_peer("S1", "127.0.0.1", 9701);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    DistributedPubSub ps1(r1, s1);
    DistributedPubSub ps2(r2, s2);
    DistributedPubSub ps3(r3, s3);

    std::vector<std::string> s2_delivered, s3_delivered;
    ps2.set_delivery_callback([&](Pid pid, const std::string&, const std::string& msg) {
        s2_delivered.push_back(std::to_string(pid.id) + ":" + msg);
    });
    ps3.set_delivery_callback([&](Pid pid, const std::string&, const std::string& msg) {
        s3_delivered.push_back(std::to_string(pid.id) + ":" + msg);
    });

    ps2.subscribe("room_1", Pid{201});
    ps3.subscribe("room_1", Pid{301});

    s1.send("S2", "PUBLISH room_1 hello_all");
    s1.send("S3", "PUBLISH room_1 hello_all");

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    assert(s2_delivered.size() == 1 && s2_delivered[0] == "201:hello_all");
    assert(s3_delivered.size() == 1 && s3_delivered[0] == "301:hello_all");

    s1.stop(); s2.stop(); s3.stop();
    std::cout << "  ✓ Test 4: 3 Node Broadcast PASS\n";

    // === Test 5: Rapid Fire 100 Messages ===
    std::cout << "[TEST 5/5] Running Rapid Fire 100 Messages test...\n";
    SocketLayer s_rf("RF", 9801);
    s_rf.start();
    RaftNode r_rf("RF", {});
    DistributedPubSub ps_rf(r_rf, s_rf);

    std::vector<std::string> rf_delivered;
    ps_rf.set_delivery_callback([&](Pid, const std::string&, const std::string& msg) {
        rf_delivered.push_back(msg);
    });

    ps_rf.subscribe("rapid", Pid{77});

    for (int i = 0; i < 100; ++i) {
        ps_rf.publish("rapid", "msg_" + std::to_string(i));
    }

    assert(rf_delivered.size() == 100);
    assert(rf_delivered[0] == "msg_0");
    assert(rf_delivered[99] == "msg_99");

    s_rf.stop();
    std::cout << "  ✓ Test 5: Rapid Fire 100 Messages PASS\n";

    std::cout << "\n🎉 ALL 5 DISTRIBUTED PUBSUB TESTS PASSED PERFECTLY!\n";
    return 0;
}
