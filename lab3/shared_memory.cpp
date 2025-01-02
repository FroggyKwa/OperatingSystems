#include "shared_memory.hpp"
#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#define GET_PID() GetCurrentProcessId()
#else
#include <unistd.h>
#define GET_PID() getpid()
#endif


const char *SHARED_MEMORY_NAME = "/shared_counter";
int *sharedCounter = nullptr;
int shm_fd;

void initializeSharedMemory() {
    shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Failed to open shared memory");
        exit(1);
    }
    ftruncate(shm_fd, sizeof(int));
    void *memory = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (memory == MAP_FAILED) {
        perror("Failed to map shared memory");
        exit(1);
    }
    sharedCounter = static_cast<int *>(memory);
    *sharedCounter = 0;
}

void cleanupSharedMemory() {
    munmap(sharedCounter, sizeof(int));
    shm_unlink(SHARED_MEMORY_NAME);
}

int *getSharedCounter() {
    return sharedCounter;
}
