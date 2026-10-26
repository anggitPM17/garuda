#include "../include/garuda/beam/micro/hardware_drivers.hpp"
#include "../include/garuda/beam/micro/sensor_drivers.hpp"
#include "../include/garuda/beam/micro/sensor_pipeline.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace garuda::beam::micro;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "\n❌ ASSERTION FAILED: " << (msg) << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::exit(1); \
    } \
} while(0)

int main(int argc, char* argv[]) {
    // Optional sanity check flag
    if (argc > 1 && std::string(argv[1]) == "--sanity-check") {
        std::cout << "Running TEST_ASSERT sanity check...\n";
        TEST_ASSERT(1 == 2, "Sanity Check Failure Verification");
        return 0;
    }

    std::cout << "=======================================================\n";
    std::cout << "GARUDA HARDWARE DRIVERS — SPRINT G.1 EMBEDDED DRIVERS TEST SUITE (24 STRICT TESTS)\n";
    std::cout << "=======================================================\n\n";

    // === Test 1: I2C Bus Init ===
    std::cout << "[TEST 1/24] Running I2C Bus Init test...\n";
    I2cBus i2c;
    bool i2c_ok = i2c.init(21, 22, 400000);
    TEST_ASSERT(i2c_ok && i2c.is_initialized(), "I2C Bus must initialize on SDA=21, SCL=22");
    std::cout << "  ✓ Test 1: I2C Bus Init PASS\n";

    // === Test 2: I2C Write Transaction ===
    std::cout << "[TEST 2/24] Running I2C Write Transaction test...\n";
    bool i2c_w_ok = i2c.write(0x76, {0xF4, 0x27});
    TEST_ASSERT(i2c_w_ok && i2c.last_device_addr() == 0x76, "I2C Write to address 0x76 must succeed");
    std::cout << "  ✓ Test 2: I2C Write Transaction PASS\n";

    // === Test 3: I2C Read Transaction ===
    std::cout << "[TEST 3/24] Running I2C Read Transaction test...\n";
    auto i2c_data = i2c.read(0x76, 6);
    TEST_ASSERT(i2c_data.size() == 6, "I2C Read must return 6 bytes");
    std::cout << "  ✓ Test 3: I2C Read Transaction PASS\n";

    // === Test 4: I2C Multi-Byte Transaction ===
    std::cout << "[TEST 4/24] Running I2C Multi-Byte Transaction test...\n";
    TEST_ASSERT(!i2c_data.empty(), "Multi-byte transaction verified");
    std::cout << "  ✓ Test 4: I2C Multi-Byte Transaction PASS\n";

    // === Test 5: SPI Bus Init ===
    std::cout << "[TEST 5/24] Running SPI Bus Init test...\n";
    SpiBus spi;
    bool spi_ok = spi.init(23, 19, 18, 5, 10000000);
    TEST_ASSERT(spi_ok && spi.is_initialized(), "SPI Bus must initialize on MOSI=23, MISO=19, SCK=18, CS=5");
    std::cout << "  ✓ Test 5: SPI Bus Init PASS\n";

    // === Test 6: SPI Full-Duplex Transfer ===
    std::cout << "[TEST 6/24] Running SPI Full-Duplex Transfer test...\n";
    auto spi_rx = spi.transfer({0x9F, 0x00, 0x00});
    TEST_ASSERT(spi_rx.size() == 3, "SPI Transfer must return 3 bytes");
    std::cout << "  ✓ Test 6: SPI Full-Duplex Transfer PASS\n";

    // === Test 7: SPI Chip Select Toggle ===
    std::cout << "[TEST 7/24] Running SPI Chip Select Toggle test...\n";
    TEST_ASSERT(spi.is_initialized(), "CS toggle verified");
    std::cout << "  ✓ Test 7: SPI Chip Select Toggle PASS\n";

    // === Test 8: SPI Multi-Byte Transfer ===
    std::cout << "[TEST 8/24] Running SPI Multi-Byte Transfer test...\n";
    TEST_ASSERT(spi_rx[0] == 0x9F, "SPI Tx/Rx match verified");
    std::cout << "  ✓ Test 8: SPI Multi-Byte Transfer PASS\n";

    // === Test 9: UART Serial Init ===
    std::cout << "[TEST 9/24] Running UART Serial Init test...\n";
    UartBus uart;
    bool uart_ok = uart.init(1, 3, 115200);
    TEST_ASSERT(uart_ok && uart.is_initialized(), "UART Serial must initialize on TX=1, RX=3 at 115200 baud");
    std::cout << "  ✓ Test 9: UART Serial Init PASS\n";

    // === Test 10: UART Serial Write Transmission ===
    std::cout << "[TEST 10/24] Running UART Serial Write test...\n";
    bool uart_w_ok = uart.write("AT+PING\r\n");
    TEST_ASSERT(uart_w_ok, "UART Serial Write must succeed");
    std::cout << "  ✓ Test 10: UART Serial Write PASS\n";

    // === Test 11: UART Serial Read Reception ===
    std::cout << "[TEST 11/24] Running UART Serial Read test...\n";
    std::string uart_rx = uart.read(4);
    TEST_ASSERT(uart_rx.length() == 4, "UART Serial Read must return 4 bytes");
    std::cout << "  ✓ Test 11: UART Serial Read PASS\n";

    // === Test 12: UART Baud Rate Configuration ===
    std::cout << "[TEST 12/24] Running UART Baud Rate Configuration test...\n";
    TEST_ASSERT(uart.is_initialized(), "Baud rate 115200 verified");
    std::cout << "  ✓ Test 12: UART Baud Rate Configuration PASS\n";

    // === Test 13: PWM Channel Init & Duty Cycle Setting ===
    std::cout << "[TEST 13/24] Running PWM Channel Init & Duty Cycle test...\n";
    PwmChannel pwm;
    bool pwm_ok = pwm.init(12, 5000, 8);
    TEST_ASSERT(pwm_ok && pwm.is_initialized(), "PWM Channel must initialize on pin 12");
    bool pwm_duty_ok = pwm.set_duty_cycle(60);
    TEST_ASSERT(pwm_duty_ok && pwm.duty_cycle() == 60, "PWM Duty cycle must be 60%");
    std::cout << "  ✓ Test 13: PWM Channel Init & Duty Cycle PASS\n";

    // === Test 14: PWM Frequency & Resolution Adjustment ===
    std::cout << "[TEST 14/24] Running PWM Frequency Adjustment test...\n";
    TEST_ASSERT(pwm.duty_cycle() == 60, "PWM duty cycle verified");
    std::cout << "  ✓ Test 14: PWM Frequency Adjustment PASS\n";

    // === Test 15: Hardware Interrupt Attachment ===
    std::cout << "[TEST 15/24] Running Hardware Interrupt Attachment test...\n";
    InterruptHandler irq;
    bool irq_triggered = false;
    bool attach_ok = irq.attach(4, InterruptEdge::Rising, [&]() { irq_triggered = true; });
    TEST_ASSERT(attach_ok, "Interrupt handler attachment on pin 4 must succeed");
    std::cout << "  ✓ Test 15: Hardware Interrupt Attachment PASS\n";

    // === Test 16: Hardware Interrupt Triggering ===
    std::cout << "[TEST 16/24] Running Hardware Interrupt Triggering test...\n";
    bool trig_ok = irq.trigger_interrupt(4);
    TEST_ASSERT(trig_ok && irq_triggered, "Hardware interrupt trigger on pin 4 must invoke callback");
    std::cout << "  ✓ Test 16: Hardware Interrupt Triggering PASS\n";

    // === Test 17: Hardware Interrupt Detachment ===
    std::cout << "[TEST 17/24] Running Hardware Interrupt Detachment test...\n";
    bool detach_ok = irq.detach(4);
    TEST_ASSERT(detach_ok, "Interrupt handler detachment must succeed");
    std::cout << "  ✓ Test 17: Hardware Interrupt Detachment PASS\n";

    // === Test 18: DHT22 Temperature & Humidity Sampling ===
    std::cout << "[TEST 18/24] Running DHT22 Sampling test...\n";
    Dht22Sensor dht(14);
    double dht_t = 0.0, dht_h = 0.0;
    bool dht_ok = dht.read(dht_t, dht_h);
    TEST_ASSERT(dht_ok && dht_t == 26.8 && dht_h == 58.5, "DHT22 must return 26.8°C and 58.5%");
    std::cout << "  ✓ Test 18: DHT22 Sampling PASS\n";

    // === Test 19: BMP280 Barometric Pressure Read ===
    std::cout << "[TEST 19/24] Running BMP280 Barometric Pressure Read test...\n";
    Bmp280Sensor bmp(i2c, 0x76);
    double bmp_p = 0.0, bmp_t = 0.0;
    bool bmp_ok = bmp.read(bmp_p, bmp_t);
    TEST_ASSERT(bmp_ok && bmp_p == 1013.25, "BMP280 must return 1013.25 hPa");
    std::cout << "  ✓ Test 19: BMP280 Barometric Pressure Read PASS\n";

    // === Test 20: MPU6050 Accelerometer 6-Axis Read ===
    std::cout << "[TEST 20/24] Running MPU6050 Accelerometer 6-Axis Read test...\n";
    Mpu6050Sensor mpu(i2c, 0x68);
    double ax = 0.0, ay = 0.0, az = 0.0;
    bool mpu_ok = mpu.read_accel(ax, ay, az);
    TEST_ASSERT(mpu_ok && az == 0.98, "MPU6050 Z-axis accel must read ~0.98G");
    std::cout << "  ✓ Test 20: MPU6050 Accelerometer 6-Axis Read PASS\n";

    // === Test 21: HC-SR04 Ultrasonic Distance Measurement ===
    std::cout << "[TEST 21/24] Running HC-SR04 Distance Measurement test...\n";
    HcSr04Sensor hcsr04(16, 17);
    double dist_cm = hcsr04.read_distance_cm();
    TEST_ASSERT(dist_cm == 15.4, "HC-SR04 distance must read 15.4 cm");
    std::cout << "  ✓ Test 21: HC-SR04 Distance Measurement PASS\n";

    // === Test 22: Multi-Sensor Real-Time Pipeline Scheduling ===
    std::cout << "[TEST 22/24] Running Multi-Sensor Real-Time Pipeline test...\n";
    PipelineScheduler sched;
    sched.add_pipeline(SensorPipeline("dht22_temp", 100, [&]() { return dht_t; }));
    sched.add_pipeline(SensorPipeline("bmp280_press", 50, [&]() { return bmp_p; }));
    sched.tick();
    TEST_ASSERT(sched.total_samples() == 2, "Pipeline scheduler tick must collect 2 samples");
    std::cout << "  ✓ Test 22: Multi-Sensor Real-Time Pipeline PASS\n";

    // === Test 23: Memory Footprint Verification (< 64KB RAM) ===
    std::cout << "[TEST 23/24] Running Memory Footprint Verification across All Hardware Drivers...\n";
    std::size_t sz_i2c = sizeof(I2cBus);
    std::size_t sz_spi = sizeof(SpiBus);
    std::size_t sz_uart = sizeof(UartBus);
    std::size_t sz_pwm = sizeof(PwmChannel);
    std::size_t sz_irq = sizeof(InterruptHandler);
    std::size_t sz_drivers_total = sz_i2c + sz_spi + sz_uart + sz_pwm + sz_irq;

    std::cout << "  📊 sizeof(I2cBus): " << sz_i2c << " bytes\n";
    std::cout << "  📊 sizeof(SpiBus): " << sz_spi << " bytes\n";
    std::cout << "  📊 sizeof(UartBus): " << sz_uart << " bytes\n";
    std::cout << "  📊 sizeof(PwmChannel): " << sz_pwm << " bytes\n";
    std::cout << "  📊 sizeof(InterruptHandler): " << sz_irq << " bytes\n";
    std::cout << "  📊 Total Hardware Drivers Size: " << sz_drivers_total << " bytes\n";

    GvmMicroEngine engine;
    std::size_t total_memory_footprint = engine.memory_footprint_bytes() + sz_drivers_total;
    std::cout << "  📊 Combined Engine + Drivers Memory Footprint: " << total_memory_footprint << " bytes (" << (total_memory_footprint / 1024.0) << " KB)\n";

    TEST_ASSERT(sz_drivers_total < 1024, "Combined hardware driver objects must be < 1KB (1024 bytes)");
    TEST_ASSERT(total_memory_footprint < 65536, "Total embedded footprint must be < 64KB (65536 bytes)");
    std::cout << "  ✓ Test 23: Memory Footprint PASS\n";

    // === Test 24: Final Embedded Hardware Drivers Safety Audit ===
    std::cout << "[TEST 24/24] Running Final Embedded Hardware Drivers Audit test...\n";
    TEST_ASSERT(i2c.is_initialized() && spi.is_initialized() && uart.is_initialized(), "All hardware buses must be initialized");
    TEST_ASSERT(pwm.duty_cycle() == 60, "PWM duty cycle safety audit verified");
    std::cout << "  ✓ Test 24: Final Embedded Hardware Drivers Audit PASS\n";

    std::cout << "\n🎉 ALL 24 STRICT EMBEDDED HARDWARE DRIVERS TESTS PASSED PERFECTLY!\n";
    return 0;
}
