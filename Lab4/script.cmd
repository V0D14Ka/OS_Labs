@echo off

set PROJECT_DIR=%~dp0
set BUILD_DIR=%PROJECT_DIR%\build

echo Обновление репозитория...
git pull
if errorlevel 1 (
    echo [ERROR] Не удалось выполнить git pull.
    exit /b 1
)

if not exist "%BUILD_DIR%" (mkdir "%BUILD_DIR%")
cd "%BUILD_DIR%"

echo Запуск сборки проекта...
cmake ..
cmake --build .
if errorlevel 1 (
    echo [ERROR] Сборка проекта завершилась с ошибкой.
    exit /b 1
)

echo Процесс завершен успешно, исполняемый файл находится в директории build.
exit /b 0
