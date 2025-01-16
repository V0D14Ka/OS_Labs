@echo off

set PROJECT_DIR=%~dp0

echo Обновление репозитория...
git pull
if errorlevel 1 (
    echo [ERROR] Не удалось выполнить git pull.
    exit /b 1
)

echo Запуск сборки проекта...
make
if errorlevel 1 (
    echo [ERROR] Сборка проекта завершилась с ошибкой.
    exit /b 1
)

echo Процесс завершен успешно.
exit /b 0
