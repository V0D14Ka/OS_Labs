#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

#ifdef _WIN32
HANDLE openSerialPort(const char* portName) {
    HANDLE hSerial = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Не удалось открыть порт: " << portName << std::endl;
        exit(1);
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Не удалось получить состояние порта." << std::endl;
        exit(1);
    }
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Не удалось установить параметры порта." << std::endl;
        exit(1);
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Не удалось установить параметры порта." << std::endl;
        exit(1);
    }

    return hSerial;
}

double readTemperatureFromPort(HANDLE hSerial) {
    DWORD bytesRead;
    char buf[256] = { 0 };
    if (ReadFile(hSerial, buf, sizeof(buf) - 1, &bytesRead, NULL)) {
        return std::strtod(buf, nullptr);
    } else {
        std::cerr << "Ошибка чтения с порта." << std::endl;
        return 0.0;
    }
}
#else
int openSerialPort(const std::string& portName) {
    int fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        std::cerr << "Не удалось открыть порт: " << portName << std::endl;
        exit(1);
    }

    struct termios options;
    tcgetattr(fd, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD; // 9600 бод, 8 бит данных
    options.c_iflag = IGNPAR; // Игнорирование параллельных ошибок
    options.c_oflag = 0;
    options.c_lflag = ICANON;
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

double readTemperatureFromPort(int fd) {
    char buf[256];
    int n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = 0; // Null-terminate string
        // Предполагаем, что данные с порта - это число
        return std::strtod(buf, nullptr);
    } else {
        std::cerr << "Ошибка чтения с порта." << std::endl;
        return 0.0;
    }
}

#endif