# KNOWN ISSUES

## TLS (Sprint B)
- [ ] TlsContext::create_client_context() memakai MBEDTLS_SSL_VERIFY_NONE.
      Risiko MITM. Wajib create_client_context(ca_pem) + VERIFY_REQUIRED
      sebelum RICHAT produksi.
- [ ] mbedTLS 3.5.2 (Jan 2024) bukan versi terbaru. Upgrade sebelum Gate Lapis 1.

## WebSocket
- [ ] tests/test_websocket_e2e.cpp menguji codec frame, bukan roundtrip
      server-client. Perbaiki sebelum menandai WebSocket [x] di ROADMAP.

## HTTP
- [ ] Audit: http_ws_server.hpp hanya handle HTTP/1.1. Klaim [x] HTTP/1.1 +
      HTTP/2 di ROADMAP perlu diverifikasi.
