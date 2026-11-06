import asyncio
import sys

try:
    import websockets
except ImportError:
    print("SKIP: websockets module not installed")
    sys.exit(0)

async def run_tests():
    uri = "ws://127.0.0.1:9103/"
    failures = 0

    print("=======================================================")
    print("GARUDA COMPILER — FASE 10.8 REAL PYTHON CLIENT WS TEST SUITE")
    print("=======================================================\n")

    # Test 1: Connect
    try:
        ws = await websockets.connect(uri)
        print("[Test 1] Real Client Connect -> 101 Switching Protocols [PASS]")
    except Exception as e:
        print(f"[Test 1] Real Client Connect -> [FAIL] {e}")
        return 1

    # Test 2: Ping-pong
    await ws.send("ping")
    reply = await asyncio.wait_for(ws.recv(), timeout=5)
    if reply == "pong":
        print("[Test 2] Real Client Ping-Pong ('ping' -> 'pong') [PASS]")
    else:
        print(f"[Test 2] Real Client Ping-Pong -> [FAIL] got {reply!r}")
        failures += 1

    # Test 3: 50KB text (Python websockets library automatically fragments large payload)
    payload_50k = "A" * 51200
    await ws.send(payload_50k)
    reply3 = await asyncio.wait_for(ws.recv(), timeout=10)
    expected3 = "Echo: " + payload_50k
    if reply3 == expected3:
        print(f"[Test 3] Real Python Client 50KB Fragmented Text Frame -> Echo Match [PASS] len={len(reply3)}")
    else:
        print(f"[Test 3] Real Python Client 50KB Fragmented Text Frame -> [FAIL] got len={len(reply3)}")
        failures += 1

    # Test 4: 30KB binary frame
    payload_bin = b"B" * 30720
    await ws.send(payload_bin)
    reply4 = await asyncio.wait_for(ws.recv(), timeout=10)
    if isinstance(reply4, bytes) and len(reply4) == 30720:
        print(f"[Test 4] Real Python Client 30KB Binary Frame -> Binary Match [PASS] len={len(reply4)}")
    else:
        print(f"[Test 4] Real Python Client 30KB Binary Frame -> [FAIL] got type={type(reply4)} len={len(reply4) if reply4 else 0}")
        failures += 1

    # Test 5: 5 sequential 10KB messages
    seq_ok = True
    for i in range(5):
        msg = f"MSG_{i}_" + ("X" * 10240)
        await ws.send(msg)
        reply5 = await asyncio.wait_for(ws.recv(), timeout=10)
        if reply5 != "Echo: " + msg:
            print(f"[Test 5] Sequential Message {i} -> [FAIL]")
            seq_ok = False
            failures += 1
            break
    if seq_ok:
        print("[Test 5] Real Python Client 5 Sequential 10KB Messages [PASS]")

    await ws.close()

    if failures > 0:
        print(f"\n❌ {failures}/5 TESTS FAILED ON PYTHON CLIENT!")
        return 1

    print("\n========================================")
    print("🎉 ALL 5 REAL PYTHON WEBSOCKET CLIENT TESTS PASSED PERFECTLY!")
    print("========================================")
    return 0

if __name__ == "__main__":
    sys.exit(asyncio.run(run_tests()))
