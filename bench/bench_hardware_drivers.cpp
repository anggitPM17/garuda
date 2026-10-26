#include "../include/garuda/beam/micro/hardware_drivers.hpp"
#include "../include/garuda/beam/micro/sensor_drivers.hpp"
#include "../include/garuda/beam/micro/sensor_pipeline.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

using namespace garuda::beam::micro;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "GARUDA HARDWARE DRIVERS BENCHMARK (SPRINT G.1 — EMBEDDED IoT PERFORMANCE)\n";
    std::cout << "=======================================================\n\n";

    // Benchmark 1: I2C Write/Read Latency (Target 40-60 µs)
    I2cBus i2c;
    i2c.init(21, 22, 400000);
    std::size_t sample_i2c = 1000;
    std::cout << "[BENCH 1] Executing " << sample_i2c << " I2C Transactions...\n";

    std::vector<double> i2c_latencies_us;
    i2c_latencies_us.reserve(sample_i2c);

    for (std::size_t i = 0; i < sample_i2c; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        i2c.write(0x76, {0xF4, 0x27});
        auto t1 = std::chrono::high_resolution_clock::now();
        i2c_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(i2c_latencies_us.begin(), i2c_latencies_us.end());
    double i2c_p50 = i2c_latencies_us[static_cast<std::size_t>(i2c_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured I2C Transaction Latency p50: " << i2c_p50 << " µs (Target 40-60 µs!)\n\n";

    // Benchmark 2: SPI Transfer Latency (Target 15-25 µs per byte)
    SpiBus spi;
    spi.init(23, 19, 18, 5, 10000000);
    std::size_t sample_spi = 1000;
    std::cout << "[BENCH 2] Executing " << sample_spi << " SPI Transfers...\n";

    std::vector<double> spi_latencies_us;
    spi_latencies_us.reserve(sample_spi);

    for (std::size_t i = 0; i < sample_spi; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        spi.transfer({0x9F});
        auto t1 = std::chrono::high_resolution_clock::now();
        spi_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(spi_latencies_us.begin(), spi_latencies_us.end());
    double spi_p50 = spi_latencies_us[static_cast<std::size_t>(spi_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured SPI Transfer Latency p50 per byte: " << spi_p50 << " µs (Target 15-25 µs!)\n\n";

    // Benchmark 3: UART Write Latency (Target 90-110 µs per byte)
    UartBus uart;
    uart.init(1, 3, 115200);
    std::size_t sample_uart = 1000;
    std::cout << "[BENCH 3] Executing " << sample_uart << " UART Transmission Operations...\n";

    std::vector<double> uart_latencies_us;
    uart_latencies_us.reserve(sample_uart);

    for (std::size_t i = 0; i < sample_uart; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        uart.write("A");
        auto t1 = std::chrono::high_resolution_clock::now();
        uart_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(uart_latencies_us.begin(), uart_latencies_us.end());
    double uart_p50 = uart_latencies_us[static_cast<std::size_t>(uart_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured UART Write Latency p50 per byte: " << uart_p50 << " µs (Target 90-110 µs!)\n\n";

    // Benchmark 4: PWM Set Duty Cycle Latency (Target <= 2.0 µs)
    PwmChannel pwm;
    pwm.init(12, 5000, 8);
    std::size_t sample_pwm = 1000;
    std::cout << "[BENCH 4] Executing " << sample_pwm << " PWM Duty Cycle Adjustments...\n";

    std::vector<double> pwm_latencies_us;
    pwm_latencies_us.reserve(sample_pwm);

    for (std::size_t i = 0; i < sample_pwm; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        pwm.set_duty_cycle(50);
        auto t1 = std::chrono::high_resolution_clock::now();
        pwm_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(pwm_latencies_us.begin(), pwm_latencies_us.end());
    double pwm_p50 = pwm_latencies_us[static_cast<std::size_t>(pwm_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured PWM Duty Cycle Latency p50: " << pwm_p50 << " µs (Target <= 2.0 µs!)\n\n";

    // Benchmark 5: Interrupt Response Latency (Target <= 5.0 µs)
    InterruptHandler irq;
    irq.attach(4, InterruptEdge::Rising, []() {});
    std::size_t sample_irq = 1000;
    std::cout << "[BENCH 5] Executing " << sample_irq << " Hardware Interrupt Triggers...\n";

    std::vector<double> irq_latencies_us;
    irq_latencies_us.reserve(sample_irq);

    for (std::size_t i = 0; i < sample_irq; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        irq.trigger_interrupt(4);
        auto t1 = std::chrono::high_resolution_clock::now();
        irq_latencies_us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
    }

    std::sort(irq_latencies_us.begin(), irq_latencies_us.end());
    double irq_p50 = irq_latencies_us[static_cast<std::size_t>(irq_latencies_us.size() * 0.50)];
    std::cout << "  ⏱️ Measured Interrupt Response Latency p50: " << irq_p50 << " µs (Target <= 5.0 µs!)\n\n";

    // Benchmark 6: Multi-Sensor Pipeline Throughput Sampling
    std::cout << "[BENCH 6] Executing Real-Time Sensor Pipeline Scheduler Throughput Sampling...\n";
    PipelineScheduler sched;
    sched.add_pipeline(SensorPipeline("dht22_temp", 10, []() { return 26.8; }));
    sched.add_pipeline(SensorPipeline("bmp280_press", 5, []() { return 1013.25; }));
    sched.add_pipeline(SensorPipeline("mpu6050_accel", 2, []() { return 0.98; }));

    auto start_sched = std::chrono::high_resolution_clock::now();
    for (int tick = 0; tick < 100; ++tick) {
        sched.tick();
    }
    auto elapsed_sched_ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - start_sched).count();

    std::size_t total_samples = sched.total_samples();
    double throughput_samples_per_sec = (total_samples / (elapsed_sched_ms > 0 ? (elapsed_sched_ms / 1000.0) : 0.001));

    std::cout << "  ⏱️ Pipeline Sampling: " << total_samples << " samples in " << elapsed_sched_ms << " ms (" << throughput_samples_per_sec << " samples/sec, Target >= 100 samples/sec!)\n\n";

    std::cout << "🎉 ALL EMBEDDED HARDWARE DRIVERS BENCHMARKS EXECUTED SUCCESSFULLY!\n";
    return 0;
}
