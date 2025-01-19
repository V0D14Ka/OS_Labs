#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include "com.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#endif


int main() {
    // Инициализация генератора случайных чисел
    std::srand(static_cast<unsigned>(std::time(nullptr)));

#ifdef _WIN32
    HANDLE serialPortFd = openSerialPort("\\\\.\\COM5");

    while (true) {
        // Генерация случайной температуры от -10.0 до 40.0
        float temperature = -10.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / 50.0f));

        // Формирование строки температуры
        std::string temperatureStr = std::to_string(temperature) + "\n";

        // Отправка данных через COM-порт
        DWORD bytesWritten;
        if (!WriteFile(serialPortFd, temperatureStr.c_str(), temperatureStr.size(), &bytesWritten, nullptr)) {
            std::cerr << "Error writing to COM port." << std::endl;
            break;
        }

        Sleep(1000);
    }

    // Закрытие COM-порта
    CloseHandle(serialPortFd);
#else
    int fd = openSerialPort("/dev/ttyUSB0");

    while (true) {
        float temperature = -10.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / 50.0f));
        std::string temperatureStr = std::to_string(temperature) + "\n";

        if (write(fd, temperatureStr.c_str(), temperatureStr.size()) == -1) {
            perror("Error writing to emulated serial port");
            break;
        }

        sleep(1);
    }

    close(fd);
#endif

    return 0;
}
