#pragma once
#include <iostream>
#include <fstream>
#include <atomic>
#include <chrono>
#include <ctime>
#include <string>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <cstring>
    #include <errno.h>
#endif

struct SharedData {
    std::atomic<int> counter;
    std::atomic<bool> isMaster{false};
#ifdef _WIN32
    CRITICAL_SECTION criticalSection;
#else
    pthread_mutex_t mutex;
#endif
};

class SharedMemory {
public:
    SharedMemory(const std::string& name) {
#ifdef _WIN32
        std::wstring wide_name(name.begin(), name.end());

        hMapFile = CreateFileMappingW(
            INVALID_HANDLE_VALUE,    
            NULL,                    
            PAGE_READWRITE,          
            0,                        
            sizeof(SharedData),      
            wide_name.c_str());

        if (hMapFile == NULL) {
            std::cerr << "Could not create file mapping object: " << GetLastError() << std::endl;
            throw std::runtime_error("Failed to create file mapping");
        }

        sharedData = (SharedData*)MapViewOfFile(
            hMapFile,                
            FILE_MAP_ALL_ACCESS,     
            0,                        
            0,                        
            sizeof(SharedData));

        if (sharedData == NULL) {
            std::cerr << "Could not map view of file: " << GetLastError() << std::endl;
            CloseHandle(hMapFile);
            throw std::runtime_error("Failed to map view of file");
        }

        InitializeCriticalSection(&sharedData->criticalSection);
#else
        int shm_fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            std::cerr << "Could not create shared memory: " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to create shared memory");
        }

        if (ftruncate(shm_fd, sizeof(SharedData)) == -1) {
            std::cerr << "Could not resize shared memory: " << strerror(errno) << std::endl;
            close(shm_fd);
            throw std::runtime_error("Failed to resize shared memory");
        }

        sharedData = (SharedData*)mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (sharedData == MAP_FAILED) {
            std::cerr << "Could not map shared memory: " << strerror(errno) << std::endl;
            close(shm_fd);
            throw std::runtime_error("Failed to map shared memory");
        }

        close(shm_fd);  // Закрываем дескриптор после маппинга

        pthread_mutex_init(&sharedData->mutex, NULL);
#endif
    }

    ~SharedMemory() {
#ifdef _WIN32
        UnmapViewOfFile(sharedData);
        CloseHandle(hMapFile);
        DeleteCriticalSection(&sharedData->criticalSection);
#else
        munmap(sharedData, sizeof(SharedData));
        pthread_mutex_destroy(&sharedData->mutex);
#endif
    }

    SharedData* get() {
        return sharedData;
    }

private:
#ifdef _WIN32
    HANDLE hMapFile;
#endif
    SharedData* sharedData;
};