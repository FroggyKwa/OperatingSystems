#include "shared_memory.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#ifdef _WIN32
#include <windows.h>
#define GET_PID() GetCurrentProcessId()
#else
#include <unistd.h>
#define GET_PID() getpid()
#endif

void logToFile(const std::string &message) {
    std::ofstream logFile("application.log", std::ios::app);
    if (!logFile) {
        std::cerr << "Failed to open log file." << std::endl;
        return;
    }
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream timestamp;
    timestamp << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
              << "." << std::setw(3) << std::setfill('0') << milliseconds.count();

    logFile << "[" << timestamp.str() << "|" << std::to_string(GET_PID()) << "] " << message << std::endl;
}

void spawnChild(int mode) {
#ifdef _WIN32
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    ZeroMemory(&processInfo, sizeof(processInfo));

    // Команда для запуска дочернего процесса
    std::string command = "child.exe " + std::to_string(mode);

    // Создание процесса
    if (CreateProcess(NULL, const_cast<char*>(command.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
        logToFile("Child process created with PID: " + std::to_string(processInfo.dwProcessId));
        WaitForSingleObject(processInfo.hProcess, INFINITE); // Ожидаем завершения
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    } else {
        logToFile("Failed to create child process. Error: " + std::to_string(GetLastError()));
    }
#else
    pid_t pid = fork();
    if (pid == 0) {
        int* counter = getSharedCounter();

        if (mode == 1) {
            *counter += 10;
            logToFile("Child 1: Incremented counter by 10.");
        } else if (mode == 2) {
            *counter *= 2;
            logToFile("Child 2: Doubled counter.");

            std::this_thread::sleep_for(std::chrono::seconds(2));

            *counter /= 2;
            logToFile("Child 2: Restored counter to original value.");
        }

        exit(0);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            logToFile("Child process [" + std::to_string(pid) + "] exited with status: " + std::to_string(WEXITSTATUS(status)));
        }
    } else {
        logToFile("Failed to create child process.");
    }
#endif
}
