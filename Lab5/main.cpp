#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include "com.hpp"
#include "sqlite3.h"

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
    ss << std::put_time(&tmStruct, "%Y-%m-%d %H:%M:%S"); // Форматируем
    return ss.str();
}

void initializeDatabase(sqlite3 *&db) {
    if (sqlite3_open("temperature_logs.db", &db)) {
        std::cerr << "Ошибка открытия базы данных: " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }

    const char *createTableQuery = "CREATE TABLE IF NOT EXISTS TemperatureLogs ("
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                   "timestamp TEXT NOT NULL,"
                                   "temperature REAL NOT NULL);";

    const char *createAvgHourTableQuery = "CREATE TABLE IF NOT EXISTS AverageHourTemperature ("
                                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                          "timestamp TEXT NOT NULL,"
                                          "average_temperature REAL NOT NULL);";

    const char *createAvgDayTableQuery = "CREATE TABLE IF NOT EXISTS AverageDayTemperature ("
                                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                         "timestamp TEXT NOT NULL,"
                                         "average_temperature REAL NOT NULL);";

    char *errMsg = nullptr;

    // Создание таблицы TemperatureLogs
    if (sqlite3_exec(db, createTableQuery, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка создания таблицы TemperatureLogs: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        exit(1);
    }

    // Создание таблицы AverageHourTemperature
    if (sqlite3_exec(db, createAvgHourTableQuery, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка создания таблицы AverageHourTemperature: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        exit(1);
    }

    // Создание таблицы AverageDayTemperature
    if (sqlite3_exec(db, createAvgDayTableQuery, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка создания таблицы AverageDayTemperature: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        exit(1);
    }
}

// Функция для очистки старых записей в базе данных для конкретной таблицы
void cleanOldLogsToDatabase(sqlite3* db, const std::string& tableName, std::chrono::hours maxAge) {
    auto now = std::chrono::system_clock::now();
    auto cutoff_time = now - maxAge;

    std::string timestampStr = formatTime(cutoff_time);
    std::string deleteQuery = "DELETE FROM " + tableName + " WHERE timestamp < '" + timestampStr + "';";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, deleteQuery.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка очистки старых записей из таблицы " << tableName << ": " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}


// Функция для записи температуры в базу данных
void logTemperatureToDatabase(sqlite3* db, const TemperatureRecord &record) {
    cleanOldLogsToDatabase(db, "TemperatureLogs", std::chrono::hours(24));
    std::string timestampStr = formatTime(record.timestamp);
    std::string query = "INSERT INTO TemperatureLogs (timestamp, temperature) VALUES ('" + timestampStr + "', " + std::to_string(record.temperature) + ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка записи в базу данных: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

// Функция для записи средней температуры за час в базу данных
void logHourlyAverageToDatabase(sqlite3* db, double averageTemperature, const std::chrono::system_clock::time_point &timestamp) {
    cleanOldLogsToDatabase(db, "AverageHourTemperature", std::chrono::hours(24 * 30));
    std::string timestampStr = formatTime(timestamp);
    std::string query = "INSERT INTO AverageHourTemperature (hour, average_temperature) VALUES ('" + timestampStr + "', " + std::to_string(averageTemperature) + ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка записи средней температуры за час в базу данных: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

// Функция для записи средней температуры за день в базу данных
void logDailyAverageToDatabase(sqlite3* db, double averageTemperature, const std::chrono::system_clock::time_point &timestamp) {
    cleanOldLogsToDatabase(db, "AverageDayTemperature", std::chrono::hours(24 * 365));
    std::string timestampStr = formatTime(timestamp);
    std::string query = "INSERT INTO AverageDayTemperature (day, average_temperature) VALUES ('" + timestampStr + "', " + std::to_string(averageTemperature) + ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка записи средней температуры за день в базу данных: " << errMsg << std::endl;
        sqlite3_free(errMsg);
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
    sqlite3* db;
    initializeDatabase(db);
    if (sqlite3_open("temperature_logs.db", &db)) {
        std::cerr << "Ошибка открытия базы данных: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

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
    long cur_day = 1;

    while (true) {
        double temperature;
        temperature = readTemperatureFromPort(serialPortFd);

        TemperatureRecord record = { std::chrono::system_clock::now(), temperature };
        temperatureRecords.push_back(record);
        logTemperatureToDatabase(db, record);

        // Каждые 60 минут записывать среднюю температуру за час
        auto now = std::chrono::system_clock::now();
        auto elapsed_hours = std::chrono::duration_cast<std::chrono::hours>(now - record.timestamp).count();
        if (elapsed_hours != 0 && elapsed_hours % 1 == 0) {  // Каждые 1 час
            double hourlyAverage = calculateHourlyAverage(temperatureRecords);
            logHourlyAverageToDatabase(db, hourlyAverage, record.timestamp);
        }

        // Каждые 24 часа записывать среднюю температуру за день
        auto elapsed_days = std::chrono::duration_cast<std::chrono::hours>(now - record.timestamp).count();
        if (elapsed_hours != 0 && elapsed_hours % 24 == 0 && elapsed_days == cur_day) {  // Каждые 24 часа
            cur_day++;
            double dailyAverage = calculateDailyAverage(temperatureRecords);
            logDailyAverageToDatabase(db, dailyAverage, record.timestamp);
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

    sqlite3_close(db);
    return 0;
}
