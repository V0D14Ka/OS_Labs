#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
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
#include "sqlite3.h"
// #include "main.cpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

struct TemperatureRecord {
    std::chrono::system_clock::time_point timestamp;
    double temperature;
};

void initializeSockets() {
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "Ошибка инициализации сокетов: " << result << std::endl;
        exit(1);
    }
#endif
}

void cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void closeSocket(int socket_fd) {
#ifdef _WIN32
    closesocket(socket_fd);
#else
    close(socket_fd);
#endif
}

std::string formatTime(const std::chrono::system_clock::time_point &timestamp) {
    auto timeT = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tmStruct = *std::localtime(&timeT);
    std::ostringstream oss;
    oss << std::put_time(&tmStruct, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}


void logTemperature(sqlite3 *db, const TemperatureRecord &record) {
    const char *insertQuery = "INSERT INTO TemperatureLogs (timestamp, temperature) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, insertQuery, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::string timestampStr = formatTime(record.timestamp);
    sqlite3_bind_text(stmt, 1, timestampStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, record.temperature);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Ошибка выполнения запроса: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

std::string getHistoryEndpoint(sqlite3 *db) {
    const char *query = "SELECT timestamp, temperature FROM TemperatureLogs ORDER BY id DESC;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        return "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nDatabase error.";
    }

    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n[";
    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) {
            response += ",";
        }
        first = false;
        response += "{";
        response += "\"timestamp\": \"" + std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0))) + "\",";
        response += "\"temperature\": " + std::to_string(sqlite3_column_double(stmt, 1));
        response += "}";
    }
    response += "]";

    sqlite3_finalize(stmt);
    return response;
}


std::string getTemperatureEndpoint(sqlite3 *db) {
    const char *query = "SELECT timestamp, temperature FROM TemperatureLogs ORDER BY id DESC LIMIT 1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        return "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nDatabase error.";
    }

    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        response += "\"timestamp\": \"" + std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0))) + "\",";
        response += "\"temperature\": " + std::to_string(sqlite3_column_double(stmt, 1));
    } else {
        response += "\"error\": \"No data available\"";
    }
    response += "}";

    sqlite3_finalize(stmt);
    return response;
}

std::string getStatsEndpoint(sqlite3 *db) {
    const char *query = "SELECT AVG(temperature) FROM TemperatureLogs WHERE timestamp >= datetime('now', '-1 day');";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        return "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nDatabase error.";
    }

    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        response += "\"average_temperature\": " + std::to_string(sqlite3_column_double(stmt, 0));
    } else {
        response += "\"error\": \"No data available\"";
    }
    response += "}";

    sqlite3_finalize(stmt);
    return response;
}

std::string handleRequest(const std::string &request, sqlite3 *db) {
    if (request.find("GET /temperature") == 0) {
        return getTemperatureEndpoint(db);
    } else if (request.find("GET /stats") == 0) {
        return getStatsEndpoint(db);
    } else if (request.find("GET /history") == 0) {
        return getHistoryEndpoint(db);
    } else {
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found";
    }
}


int main() {
#ifdef _WIN32
    const char* cmd_name = "cmd /c timeout /t 5 >nul 2>&1";
    SetConsoleOutputCP(CP_UTF8);
#else
    const char* cmd_name = "sleep 5";
#endif
    initializeSockets();
    sqlite3 *db;
    if (sqlite3_open("temperature_logs.db", &db)) {
        std::cerr << "Ошибка открытия базы данных: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    const int PORT = 8080;

#ifdef _WIN32
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета: " << WSAGetLastError() << std::endl;
        cleanupSockets();
        return -1;
    }
#else
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Ошибка создания сокета");
        return -1;
    }
#endif

    // Настраиваем параметры сокета
#ifdef _WIN32
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "Ошибка настройки сокета: " << WSAGetLastError() << std::endl;
        closeSocket(server_fd);
        cleanupSockets();
        return -1;
    }
#else
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Ошибка настройки сокета");
        closeSocket(server_fd);
        return -1;
    }
#endif

    // Заполняем структуру адреса
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Привязываем сокет к адресу и порту
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
#ifdef _WIN32
        std::cerr << "Ошибка привязки: " << WSAGetLastError() << std::endl;
#else
        perror("Ошибка привязки");
#endif
        closeSocket(server_fd);
        cleanupSockets();
        return -1;
    }

    // Начинаем прослушивание
    if (listen(server_fd, 3) < 0) {
#ifdef _WIN32
        std::cerr << "Ошибка прослушивания: " << WSAGetLastError() << std::endl;
#else
        perror("Ошибка прослушивания");
#endif
        closeSocket(server_fd);
        cleanupSockets();
        return -1;
    }

    std::cout << "Сервер запущен на порту " << PORT << std::endl;

    while (true) {
        int addrlen = sizeof(address);
        int client_socket;
#ifdef _WIN32
        client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Ошибка принятия соединения: " << WSAGetLastError() << std::endl;
#else
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket < 0) {
            perror("Ошибка принятия соединения");
#endif
            closeSocket(server_fd);
            cleanupSockets();
            sqlite3_close(db);
            return -1;
        }

        std::cout << "Соединение принято" << std::endl;

        char buffer[1024] = {0};
        int valread;
#ifdef _WIN32
        valread = recv(client_socket, buffer, sizeof(buffer), 0);
#else
        valread = read(client_socket, buffer, sizeof(buffer));
#endif

        if (valread > 0) {
            std::string request(buffer, valread);
            std::cout << "Получен запрос: \n" << request << std::endl;

            std::string response = handleRequest(request, db);
#ifdef _WIN32
            send(client_socket, response.c_str(), response.length(), 0);
#else
            send(client_socket, response.c_str(), response.length(), 0);
#endif
        }

        closeSocket(client_socket);
    }

    closeSocket(server_fd);
    cleanupSockets();
    sqlite3_close(db);

    return 0;
}
