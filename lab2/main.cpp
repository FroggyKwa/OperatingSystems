#include "background.hpp"
#include <iostream>

int main() {
    try {
        const char* program_path =
    #ifdef _WIN32
        "notepad.exe";
    #else
        "/bin/ls";
    #endif

        int status = 0;
        int pid = start_background(program_path, status);
        std::cout << "Запущен процесс с PID: " << pid << std::endl;

        int exit_code = 0;
        wait_program(pid, &exit_code);
        std::cout << "Процесс завершился с кодом: " << exit_code << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
