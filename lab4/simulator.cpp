#include "my_serial.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <string>
#include <chrono>

constexpr const char* DEFAULT_PORT = "COM4";
constexpr double NORMAL_MEAN = 0.0;
constexpr double NORMAL_STDDEV = 8.0;
constexpr std::chrono::seconds SEND_INTERVAL(1);

std::string GenerateRandomData(std::mt19937& generator, std::normal_distribution<>& distribution) {
    double random_value = distribution(generator);
    return "$$" + std::to_string(random_value) + "$$";
}

bool InitializeSerialPort(cplib::SerialPort& serial_port, const std::string& port) {
    if (!serial_port.IsOpen()) {
        std::cerr << "Failed to open port: " << port << std::endl;
        return false;
    }
    std::cout << "Port opened successfully: " << port << std::endl;
    return true;
}

void StartSendingData(cplib::SerialPort& serial_port) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> distribution(NORMAL_MEAN, NORMAL_STDDEV);

    while (true) {
        std::string random_data = GenerateRandomData(gen, distribution);
        serial_port << random_data;
        std::cout << "Sent: " << random_data << std::endl;
        std::this_thread::sleep_for(SEND_INTERVAL);
    }
}

int main(int argc, char** argv) {
    std::string port = (argc > 1) ? argv[1] : DEFAULT_PORT;

    cplib::SerialPort serial_port(port, cplib::SerialPort::BAUDRATE_115200);

    if (!InitializeSerialPort(serial_port, port)) {
        std::cout
        << "Arguments: [port];"
        << std::endl
        << "Default: "
        << DEFAULT_PORT
        << std::endl;
        return -2;
    }

    StartSendingData(serial_port);

    return 0;
}
