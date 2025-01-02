#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

void initializeSharedMemory();

void cleanupSharedMemory();

int *getSharedCounter();

#endif // SHARED_MEMORY_HPP