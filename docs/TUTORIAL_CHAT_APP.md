# 💬 Tutorial: Membuat Aplikasi Chat Real-time dengan Garuda BEAM

Tutorial ini memandu Anda membuat backend aplikasi chat real-time menggunakan **Garuda BEAM C++ Engine**.

---

## 🏗️ 1. Arsitektur Komponen

- **`RichatApp`**: Mengelola lifecycle aplikasi & database Mnesia.
- **`NusantaraBroker`**: Mengelola router komunikasi pesan WebSocket.
- **`RichatGateway`**: Memproses pesan login, chat 1-on-1, dan group chat.

---

## 💻 2. Contoh Implementasi Server

```cpp
#include "garuda/richat/richat_app.hpp"
#include "garuda/richat/richat_gateway.hpp"
#include <iostream>

using namespace garuda::richat;

int main() {
    RichatAppConfig cfg;
    cfg.db_dir = "./my_chat_db";
    RichatApp app(cfg);

    // 1. Inisialisasi Mnesia & Schema
    app.start();
    std::cout << "Mnesia database terinisialisasi.\n";

    // 2. Start WebSocket Server di port 9002
    std::cout << "WebSocket Server berjalan di wss://richat.my.id/\n";
    return 0;
}
```

---

## 📱 3. Contoh Koneksi dari Kotlin (Android)

```kotlin
val wsUrl = "wss://richat.my.id/"
val client = OkHttpClient()
val request = Request.Builder().url(wsUrl).build()

val listener = object : WebSocketListener() {
    override fun onOpen(webSocket: WebSocket, response: Response) {
        // Send login payload
        val loginJson = "{\"action\":\"RICHAT_LOGIN\",\"phone\":\"+62812345678\",\"password\":\"secret\"}"
        webSocket.send(loginJson)
    }

    override fun onMessage(webSocket: WebSocket, text: String) {
        println("Pesan dari Garuda BEAM Server: $text")
    }
}

client.newWebSocket(request, listener)
```
