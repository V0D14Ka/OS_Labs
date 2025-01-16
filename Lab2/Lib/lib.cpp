#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include "lib.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace ProcessManager {

    // Функция для запуска процесса в фоновом режиме
    int runBackgroundProcess(const std::string& command) {
        int exitCode = -1;

        #ifdef _WIN32
        // Для Windows используем CreateProcessW (для wide строк)
        std::wstring wcommand(command.begin(), command.end());  // Преобразуем строку в wide string
        STARTUPINFOW si = { sizeof(STARTUPINFOW) };  // Используем STARTUPINFOW для широких строк
        PROCESS_INFORMATION pi;
        
        if (!CreateProcessW(
            nullptr,               // Программа
            &wcommand[0],          // Командная строка (широкий символ)
            nullptr,               // Атрибуты процесса
            nullptr,               // Атрибуты потока
            false,                 // Наследование дескрипторов
            CREATE_NO_WINDOW,      // Запуск без окна
            nullptr,               // Среда
            nullptr,               // Рабочий каталог
            &si,                   // Структура информации о запуске
            &pi                    // Структура информации о процессе
        )) {
            throw std::runtime_error("Ошибка запуска процесса");
        }

        // Ждем завершения процесса и получаем код возврата
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, (LPDWORD)&exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        #else
        // Для POSIX систем (Linux, macOS)
        pid_t pid = fork();
        if (pid < 0) {
            throw std::runtime_error("Ошибка при создании процесса");
        } else if (pid == 0) {
            // Дочерний процесс
            int result = system(command.c_str());
            exit(result);  // Возвращаем код завершения
        } else {
            // Родительский процесс
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                exitCode = WEXITSTATUS(status);
            } else {
                throw std::runtime_error("Процесс завершился с ошибкой");
            }
        }
        #endif

        return exitCode;
    }

}
