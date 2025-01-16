#include <iostream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif
#include "Lib/lib.hpp"

int main() {

    #ifdef _WIN32
    const char* cmd_name = "cmd /c timeout /t 5 >nul 2>&1";
    SetConsoleOutputCP(CP_UTF8);
    #else
    const char* cmd_name = "sleep 5";
    #endif

    std::string command;
    std::cout << "Введите команду для выполнения в фоновом режиме: ";
    std::getline(std::cin, command);

    try {
        int exitCode = ProcessManager::runBackgroundProcess(command);
        std::cout << "Процесс завершен с кодом: " << exitCode << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}
