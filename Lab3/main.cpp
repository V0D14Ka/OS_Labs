#include <iostream>
#include <fstream>
#include <atomic>
#include <chrono>
#include <ctime>
#include <string>
#include "shared_memory.hpp"
#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <unistd.h>
    <cstring>
    #include <errno.h>
#endif

// Объявление функций spawnChild1 и spawnChild2
#ifdef _WIN32
DWORD WINAPI spawnChild1(LPVOID param);
DWORD WINAPI spawnChild2(LPVOID param);
#else
void* spawnChild1(void* param);
void* spawnChild2(void* param);
#endif

// Функция для записи в лог
void writeLog(const std::string& message) {
    std::ofstream logFile("logfile.txt", std::ios::out | std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file!" << std::endl;
    }
}

// Функция для получения текущего времени в строковом формате
std::string getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char buffer[26];
    struct tm* tm_info;
    tm_info = localtime(&time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buffer);
}

// Поток, который увеличивает счетчик
#ifdef _WIN32
DWORD WINAPI timer(LPVOID param) {
#else
void* timer(void* param) {
#endif
    SharedData* sharedData = (SharedData*)param;

    while (true) {
#ifdef _WIN32
        Sleep(300);  // Windows Sleep
#else
        sleep(1);  // POSIX sleep (пауза 1 секунда)
#endif

#ifdef _WIN32
        EnterCriticalSection(&sharedData->criticalSection);
#else
        pthread_mutex_lock(&sharedData->mutex);
#endif
        sharedData->counter++;  // Увеличиваем счетчик
#ifdef _WIN32
        LeaveCriticalSection(&sharedData->criticalSection);
#else
        pthread_mutex_unlock(&sharedData->mutex);
#endif
    }
#ifdef _WIN32
    return 0;
#else
    return nullptr;
#endif
}

// Поток для записи времени в лог
#ifdef _WIN32
DWORD WINAPI logTime(LPVOID param) {
#else
void* logTime(void* param) {
#endif
    SharedData* sharedData = (SharedData*)param;

    while (true) {
#ifdef _WIN32
        Sleep(1000);  // Windows Sleep
#else
        sleep(1);  // POSIX sleep (пауза 1 секунда)
#endif
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        // Используем стандартную функцию ctime
        char buffer[26];
        struct tm* tm_info;
        tm_info = localtime(&time);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

#ifdef _WIN32
        EnterCriticalSection(&sharedData->criticalSection);
#else
        pthread_mutex_lock(&sharedData->mutex);
#endif
        writeLog("Time: " + std::string(buffer) + " Process ID: " + std::to_string(GetCurrentProcessId()) + " Counter: " + std::to_string(sharedData->counter.load()));
#ifdef _WIN32
        LeaveCriticalSection(&sharedData->criticalSection);
#else
        pthread_mutex_unlock(&sharedData->mutex);
#endif
    }
#ifdef _WIN32
    return 0;
#else
    return nullptr;
#endif
}

// Поток для мониторинга дочерних процессов
#ifdef _WIN32
DWORD WINAPI monitorChildren(LPVOID param) {
#else
void* monitorChildren(void* param) {
#endif
    SharedData* sharedData = (SharedData*)param;
    while (true) {
#ifdef _WIN32
        Sleep(3000);  // Windows Sleep
#else
        sleep(3);  // POSIX sleep (3 секунды)
#endif

        // Запускаем Child1
#ifdef _WIN32
        HANDLE child1 = CreateThread(nullptr, 0, spawnChild1, sharedData, 0, nullptr);
        if (child1 != nullptr) {
            WaitForSingleObject(child1, INFINITE);
            CloseHandle(child1);
        }
#else
        pthread_t child1;
        pthread_create(&child1, nullptr, spawnChild1, sharedData);
        pthread_join(child1, nullptr);
#endif

        // Запускаем Child2
#ifdef _WIN32
        HANDLE child2 = CreateThread(nullptr, 0, spawnChild2, sharedData, 0, nullptr);
        if (child2 != nullptr) {
            WaitForSingleObject(child2, INFINITE);
            CloseHandle(child2);
        }
#else
        pthread_t child2;
        pthread_create(&child2, nullptr, spawnChild2, sharedData);
        pthread_join(child2, nullptr);
#endif
    }
#ifdef _WIN32
    return 0;
#else
    return nullptr;
#endif
}

// Поток для первого дочернего процесса
#ifdef _WIN32
DWORD WINAPI spawnChild1(LPVOID param) {
#else
void* spawnChild1(void* param) {
#endif
    SharedData* sharedData = (SharedData*)param;
    auto start_time = std::chrono::system_clock::now();
    std::string start_time_str = getCurrentTimeString();
    writeLog("Child1 started at " + start_time_str + ", PID: " + std::to_string(GetCurrentProcessId()));

#ifdef _WIN32
    EnterCriticalSection(&sharedData->criticalSection);
#else
    pthread_mutex_lock(&sharedData->mutex);
#endif
    sharedData->counter.fetch_add(10);  // Атомарное увеличение на 10
    writeLog("Child1 added counter by 10, new value: " + std::to_string(sharedData->counter.load()));
#ifdef _WIN32
    LeaveCriticalSection(&sharedData->criticalSection);
#else
    pthread_mutex_unlock(&sharedData->mutex);
#endif

    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    std::string end_time_str = getCurrentTimeString();
    writeLog("Child1 finished at " + end_time_str + ", PID: " + std::to_string(GetCurrentProcessId()) + " Duration: " + std::to_string(duration) + " seconds");

#ifdef _WIN32
    return 0;
#else
    return nullptr;
#endif
}

// Поток для второго дочернего процесса
#ifdef _WIN32
DWORD WINAPI spawnChild2(LPVOID param) {
#else
void* spawnChild2(void* param) {
#endif
    SharedData* sharedData = (SharedData*)param;
    auto start_time = std::chrono::system_clock::now();
    std::string start_time_str = getCurrentTimeString();
    writeLog("Child2 started at " + start_time_str + ", PID: " + std::to_string(GetCurrentProcessId()));

    // Блокируем только для изменения счетчика (умножение на 2)
#ifdef _WIN32
    EnterCriticalSection(&sharedData->criticalSection);
#else
    pthread_mutex_lock(&sharedData->mutex);
#endif
    sharedData->counter.store(sharedData->counter.load() * 2);  // Атомарное умножение на 2
    writeLog("Child2 multiplied counter by 2, new value: " + std::to_string(sharedData->counter.load()));
#ifdef _WIN32
    LeaveCriticalSection(&sharedData->criticalSection);
#else
    pthread_mutex_unlock(&sharedData->mutex);
#endif

    // Эмуляция ожидания 2 секунд без блокировки счетчика
    Sleep(2000);  // Эмуляция ожидания 2 секунды

    // Делим счетчик на 2, снова блокируем для атомарной операции
#ifdef _WIN32
    EnterCriticalSection(&sharedData->criticalSection);
#else
    pthread_mutex_lock(&sharedData->mutex);
#endif
    sharedData->counter.store(sharedData->counter.load() / 2);  // Атомарное деление на 2
    writeLog("Child2 divided counter by 2, new value: " + std::to_string(sharedData->counter.load()));
#ifdef _WIN32
    LeaveCriticalSection(&sharedData->criticalSection);
#else
    pthread_mutex_unlock(&sharedData->mutex);
#endif

    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    std::string end_time_str = getCurrentTimeString();
    writeLog("Child2 finished at " + end_time_str + ", PID: " + std::to_string(GetCurrentProcessId()) + " Duration: " + std::to_string(duration) + " seconds");

#ifdef _WIN32
    return 0;
#else
    return nullptr;
#endif
}

int main(int argc, char* argv[]) {
    try {
        SharedMemory sharedMemory("SharedMemory");
        SharedData* sharedData = sharedMemory.get();
        sharedData->counter.store(0);

        // Записываем начальную информацию в лог
        auto now = std::chrono::system_clock::now();
        auto start_time = std::chrono::system_clock::to_time_t(now);
        writeLog("Program started, PID: " + std::to_string(GetCurrentProcessId()) + " Start time: " + ctime(&start_time));

#ifdef _WIN32
        CreateThread(nullptr, 0, timer, sharedData, 0, nullptr);
        CreateThread(nullptr, 0, logTime, sharedData, 0, nullptr);
        CreateThread(nullptr, 0, monitorChildren, sharedData, 0, nullptr);
#else
        pthread_t timerThread, logTimeThread, monitorThread;
        pthread_create(&timerThread, nullptr, timer, sharedData);
        pthread_create(&logTimeThread, nullptr, logTime, sharedData);
        pthread_create(&monitorThread, nullptr, monitorChildren, sharedData);

        pthread_join(timerThread, nullptr);
        pthread_join(logTimeThread, nullptr);
        pthread_join(monitorThread, nullptr);
#endif

        // Интерфейс командной строки для установки значения счетчика
        while (true) {
            std::cout << "Enter a value for the counter (or 'exit' to quit): ";
            std::string input;
            std::cin >> input;

            if (input == "exit") {
                break;
            }

            try {
                int new_value = std::stoi(input);
                sharedData->counter.store(new_value);
            } catch (const std::invalid_argument&) {
                std::cout << "Invalid input. Please enter a valid number." << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
