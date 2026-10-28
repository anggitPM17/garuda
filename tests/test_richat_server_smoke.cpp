// =====================================================================
// RICHAT SERVER SMOKE TEST
// Memverifikasi bahwa RichatGateway + NusantaraBroker bisa diinisialisasi
// (tanpa benar-benar listen di port, untuk CI safety)
// =====================================================================
#include "../include/garuda/richat/richat_gateway.hpp"
#include "../include/garuda/nusantara/nusantara_broker.hpp"
#include "../include/garuda/jalur_langit/jalur_langit_ultimate.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <system_error>

using namespace garuda::richat;
using namespace garuda::nusantara;
using namespace garuda::jalur_langit;

#define CHECK(cond, msg) if (!(cond)) { std::cerr << "FAILED: " << msg << "\n"; std::exit(1); }

int main() {
    std::cout << "========================================\n";
    std::cout << "RICHAT SERVER SMOKE TEST\n";
    std::cout << "========================================\n\n";

    std::string test_dir = "./test_richat_server_smoke";
    std::error_code ec;
    std::filesystem::remove_all(test_dir, ec);

    {
        // 1. Inisialisasi database
        JalurLangitUltimate db(test_dir);
        std::cout << "[1] Database initialized\n";

        // 2. Inisialisasi gateway
        RichatGateway gateway(db);
        std::cout << "[2] Gateway initialized\n";

        // 3. Register entity
        NusantaraBroker::get_instance().register_entity(
            EntityTarget::JALUR_LANGIT,
            [&gateway](const NusantaraMessage& msg) {
                gateway.handle_message(msg);
            }
        );
        std::cout << "[3] Entity registered\n";

        // 4. Start worker (TANPA listener, agar CI aman)
        NusantaraBroker::get_instance().start();
        std::cout << "[4] Worker started (no listener)\n";

        // 5. Kirim pesan login via in-process (tidak lewat network)
        NusantaraMessage login_msg;
        login_msg.message_id = "smoke_login";
        login_msg.sender = EntityTarget::RICHAT_APP;
        login_msg.receiver = EntityTarget::JALUR_LANGIT;
        login_msg.action = NusantaraAction::RICHAT_LOGIN;
        login_msg.payload["auth_token"] = "token_abc";
        login_msg.payload["device_id"] = "android_001";

        // 6. Langsung panggil gateway (bypass network, simulasi)
        gateway.handle_message(login_msg);
        NusantaraBroker::get_instance().process_queue();
        std::cout << "[5] Login message processed\n";

        // 7. Verifikasi metrics
        std::string metrics = gateway.get_metrics();
        CHECK(metrics.find("richat_requests_total 1") != std::string::npos,
              "Harus ada 1 request tercatat");
        std::cout << "[6] Metrics verified\n";

        // 8. Stop worker
        NusantaraBroker::get_instance().stop();
        std::cout << "[7] Worker stopped\n";
    }

    std::filesystem::remove_all(test_dir, ec);

    std::cout << "\n========================================\n";
    std::cout << "RICHAT SERVER SMOKE TEST PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
