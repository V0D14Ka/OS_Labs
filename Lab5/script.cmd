@echo off

set PROJECT_DIR=%~dp0
set BUILD_DIR=%PROJECT_DIR%\build
set PYVENV_DIR=%PROJECT_DIR%\venv

echo Обновление репозитория...
git pull
if errorlevel 1 (
    echo [ERROR] Не удалось выполнить git pull.
    exit /b 1
)

if not exist "%BUILD_DIR%" (mkdir "%BUILD_DIR%")
if not exist "%PYVENV_DIR%" (
    python -m venv venv
)

call ./venv/Scripts/activate
pip install -r requirements.txt

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
