#include "process_handler.hpp"
#include "shared_memory.hpp"
#include <cstdarg>
#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <chrono>
#include <string>


#ifdef _WIN32
#include <windows.h>
#define GET_PID() GetCurrentProcessId()
#else
#include <unistd.h>
#define GET_PID() getpid()
#endif

int main() {
    initializeSharedMemory();
    int *counter = getSharedCounter();

    logToFile("Main process started. PID:" + std::to_string(GET_PID()));

    std::thread timerThread([&]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            (*counter)++;
        }
    });

    std::thread loggerThread([&]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            logToFile("[LOG] Counter: " + std::to_string(*counter));
        }
    });

    std::thread childSpawnerThread([&]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            spawnChild(1);
            spawnChild(2);
        }
    });
    std::string param;

    while (true)
        {
            std::cin >> param;
            if (param == "exit")
            {
                exit(0);
            }
            else if (param == "change")
            {
                int value;
                std::cout << "Enter number to set counter to\n";
                std::cin >> value;
                *counter = value;
            }
            else if (param == "show")
            {
                std::cout << "Counter value: " << *counter << "\n";
            }
        }

    timerThread.join();
    loggerThread.join();
    childSpawnerThread.join();

    cleanupSharedMemory();
    std::cout << "Exiting...\n";
    return 0;
}
