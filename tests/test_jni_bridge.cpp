// =====================================================================
// FASE D.4.4 - RICHAT JNI BRIDGE TESTS
// Memverifikasi jni_bridge.cpp
// =====================================================================
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdint>

extern "C" {
std::int64_t jni_native_ws_create();
bool jni_native_ws_connect(std::int64_t ptr, const char* url);
bool jni_native_ws_send(std::int64_t ptr, const char* text);
void jni_native_ws_close(std::int64_t ptr);
void jni_native_ws_destroy(std::int64_t ptr);

const char* jni_native_crypto_sha256(const char* input);

std::int64_t jni_native_sync_create(const char* db_path);
bool jni_native_sync_save(std::int64_t ptr, const char* id, const char* sender, const char* recipient, const char* content, std::int64_t timestamp);
int jni_native_sync_pending_count(std::int64_t ptr);
void jni_native_sync_destroy(std::int64_t ptr);
}

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "FASE D.4.4 - JNI BRIDGE TESTS\n";
    std::cout << "========================================\n\n";

    // Test 1: Native WebSocket JNI Lifecycle
    std::cout << "[Test 1] Native WebSocket JNI Lifecycle...\n";
    {
        std::int64_t ws_ptr = jni_native_ws_create();
        CHECK(ws_ptr != 0, "jni_native_ws_create harus return pointer valid");

        bool connected = jni_native_ws_connect(ws_ptr, "wss://richat.my.id/");
        CHECK(connected, "jni_native_ws_connect harus return true");

        bool sent = jni_native_ws_send(ws_ptr, "Hello JNI");
        CHECK(sent, "jni_native_ws_send harus return true");

        jni_native_ws_close(ws_ptr);
        jni_native_ws_destroy(ws_ptr);
    }
    std::cout << "PASS\n\n";

    // Test 2: Native Crypto JNI SHA256
    std::cout << "[Test 2] Native Crypto JNI SHA256...\n";
    {
        const char* hash = jni_native_crypto_sha256("garuda_jni");
        CHECK(hash != nullptr && std::string(hash).length() == 64, "JNI SHA256 length harus 64 hex chars");
    }
    std::cout << "PASS\n\n";

    // Test 3: Native Sync JNI Lifecycle
    std::cout << "[Test 3] Native Sync JNI Lifecycle...\n";
    {
        std::int64_t sync_ptr = jni_native_sync_create(":memory:");
        CHECK(sync_ptr != 0, "jni_native_sync_create harus return pointer valid");

        bool saved = jni_native_sync_save(sync_ptr, "jni_m1", "usr_a", "usr_b", "JNI Sync Msg", 1700000000);
        CHECK(saved, "jni_native_sync_save harus return true");

        int count = jni_native_sync_pending_count(sync_ptr);
        CHECK(count == 1, "jni_native_sync_pending_count harus 1");

        jni_native_sync_destroy(sync_ptr);
    }
    std::cout << "PASS\n\n";

    // Test 4: Null Pointer Rejection
    std::cout << "[Test 4] Null Pointer Rejection...\n";
    {
        CHECK(!jni_native_ws_connect(0, "wss://richat.my.id/"), "Null ptr connect harus return false");
        CHECK(!jni_native_ws_send(0, "test"), "Null ptr send harus return false");
        CHECK(jni_native_sync_pending_count(0) == 0, "Null ptr sync pending_count harus 0");
    }
    std::cout << "PASS\n\n";

    // Test 5: Null String Rejection
    std::cout << "[Test 5] Null String Rejection...\n";
    {
        std::int64_t sync_ptr = jni_native_sync_create(":memory:");
        bool saved = jni_native_sync_save(sync_ptr, nullptr, nullptr, nullptr, nullptr, 0);
        CHECK(!saved, "jni_native_sync_save dengan ID null harus return false");
        jni_native_sync_destroy(sync_ptr);
    }
    std::cout << "PASS\n\n";

    std::cout << "========================================\n";
    std::cout << "FASE D.4.4 - JNI BRIDGE SUITE PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
