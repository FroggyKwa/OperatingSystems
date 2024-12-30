#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif


int start_background(const char *program_path) {
#ifdef _WIN32
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    if (!CreateProcess(nullptr, const_cast<char*>(program_path), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        throw std::runtime_error("Failed to start process");
    }

    CloseHandle(pi.hThread);
    return static_cast<int>(pi.dwProcessId);
#else
    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("Failed to fork process");
    } else if (pid == 0) {
        execlp(program_path, program_path, nullptr);
        _exit(127);
    }
    return static_cast<int>(pid);
#endif
}

int start_background(const char *program_path, int &status) {
    int pid = start_background(program_path);
    status = 0;
    return pid;
}

int wait_program(const int pid, int* exit_code) {
#ifdef _WIN32
    HANDLE process_handle = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(pid));
    if (!process_handle) {
        throw std::runtime_error("Failed to open process handle");
    }

    WaitForSingleObject(process_handle, INFINITE);

    DWORD exit_status;
    if (GetExitCodeProcess(process_handle, &exit_status)) {
        if (exit_code) {
            *exit_code = static_cast<int>(exit_status);
        }
    } else {
        CloseHandle(process_handle);
        throw std::runtime_error("Failed to get process exit code");
    }

    CloseHandle(process_handle);
    return 0;
#else
    int status;
    pid_t result = waitpid(pid, &status, 0);
    if (result == -1) {
        throw std::runtime_error("Failed to wait for process");
    }

    if (exit_code && WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
    }
    return 0;
#endif
}
