#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include "com.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

struct TemperatureRecord {
    std::chrono::system_clock::time_point timestamp;
    double temperature;
};

// Функция для форматированного вывода времени
std::string formatTime(const std::chrono::system_clock::time_point &timestamp) {
    auto timeT = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tmStruct = *std::localtime(&timeT);  // Преобразуем в struct tm
    std::stringstream ss;
    ss << std::put_time(&tmStruct, "%a %b %d %H:%M:%S %Y"); // Форматируем
    return ss.str();
}

// Функция для очистки старых записей в логе
void cleanOldLogs(const std::string &filename, std::chrono::hours maxAge) {
    std::ifstream file(filename);
    std::vector<std::string> lines;
    std::string line;
    auto now = std::chrono::system_clock::now();

    // Читаем все строки из файла
    while (std::getline(file, line)) {
        std::time_t timestamp;
        std::tm tmStruct = {};
        
        // Преобразуем строку в timestamp (предположим, что формат времени соответствует ctime())
        std::istringstream ss(line);
        ss >> std::get_time(&tmStruct, "%a %b %d %H:%M:%S %Y");

        // Если строка не может быть преобразована в дату, пропускаем её
        if (ss.fail()) {
            continue;
        }

        timestamp = std::mktime(&tmStruct);
        auto recordTime = std::chrono::system_clock::from_time_t(timestamp);

        // Если запись слишком старая, пропускаем её
        if (std::chrono::duration_cast<std::chrono::hours>(now - recordTime) <= maxAge) {
            lines.push_back(line);
        }
    }

    // Перезаписываем файл с актуальными данными
    std::ofstream outFile(filename);
    for (const auto &line : lines) {
        outFile << line << "\n";
    }
}


// Функция для записи температуры в лог
void logTemperature(const std::string &filename, const TemperatureRecord &record) {
    cleanOldLogs(filename, std::chrono::hours(24));
    std::ofstream file(filename, std::ios_base::app);
    if (file.is_open()) {
        file << formatTime(record.timestamp) << ": " << record.temperature << "\n";
    }
}

// Функция для вычисления средней температуры за последний час
double calculateHourlyAverage(const std::vector<TemperatureRecord> &records) {
    double total = 0;
    int count = 0;
    auto now = std::chrono::system_clock::now();
    for (const auto &record : records) {
        auto elapsed_hours = std::chrono::duration_cast<std::chrono::hours>(now - record.timestamp).count();
        if (elapsed_hours <= 1) {
            total += record.temperature;
            count++;
        }
    }
    return count > 0 ? total / count : 0;
}

// Функция для вычисления средней температуры за последний день
double calculateDailyAverage(const std::vector<TemperatureRecord> &records) {
    double total = 0;
    int count = 0;
    auto now = std::chrono::system_clock::now();
    for (const auto &record : records) {
        auto elapsed_hours = std::chrono::duration_cast<std::chrono::hours>(now - record.timestamp).count();
        if (elapsed_hours <= 24) {
            total += record.temperature;
            count++;
        }
    }
    return count > 0 ? total / count : 0;
}


int main() {
    std::vector<TemperatureRecord> temperatureRecords;
#ifdef _WIN32
    const char* cmd_name = "cmd /c timeout /t 5 >nul 2>&1";
    SetConsoleOutputCP(CP_UTF8);
#else
    const char* cmd_name = "sleep 5";
#endif

#ifdef _WIN32
    HANDLE serialPortFd = openSerialPort("\\\\.\\COM6");
#else
    int serialPortFd = openSerialPort("/dev/ttyUSB0");
#endif

    auto lastReadTime = std::chrono::steady_clock::now();

    while (true) {
        double temperature;
        long cur_day = 1;
        temperature = readTemperatureFromPort(serialPortFd);

        //std::cout << temperature << "\n";
        TemperatureRecord record = { std::chrono::system_clock::now(), temperature };
        temperatureRecords.push_back(record);
        logTemperature("temperature_log.txt", record);

        // Каждые 60 минут записывать среднюю температуру за час
        auto now = std::chrono::system_clock::now();
        auto elapsed_hours = std::chrono::duration_cast<std::chrono::hours>(now - record.timestamp).count();
        if (elapsed_hours != 0 and elapsed_hours % 1 == 0) {  // Каждые 1 час
            cleanOldLogs("hourly_average_log.txt", std::chrono::hours(24 * 30));
            double hourlyAverage = calculateHourlyAverage(temperatureRecords);
            std::ofstream hourlyFile("hourly_average_log.txt", std::ios_base::app);
            hourlyFile << formatTime(record.timestamp) << ": " << hourlyAverage << "\n";
        }

        // Каждые 24 часа записывать среднюю температуру за день
        auto elapsed_days = std::chrono::duration_cast<std::chrono::hours>(now - record.timestamp).count();
        if (elapsed_hours != 0 and elapsed_hours % 24 == 0 and elapsed_days == cur_day) {  // Каждые 24 часа
            cur_day++;
            cleanOldLogs("daily_average_log.txt", std::chrono::hours(24 * 365));
            double dailyAverage = calculateDailyAverage(temperatureRecords);
            std::ofstream dailyFile("daily_average_log.txt", std::ios_base::app);
            dailyFile << formatTime(record.timestamp) << ": " << dailyAverage << "\n";
        }
#ifdef _WIN32
        Sleep(300);
#else
        sleep(1);
#endif
    }

#ifdef __linux__
    close(serialPortFd);
#elif _WIN32
    CloseHandle(serialPortFd);
#endif
    return 0;
}
