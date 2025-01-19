#!/bin/bash

PROJECT_DIR=$(dirname "$(realpath "$0")")
BUILD_DIR="$PROJECT_DIR/build"
PYVENV_DIR="$PROJECT_DIR/venv"

echo "Обновление репозитория..."
git pull
if [ $? -ne 0 ]; then
    echo "[ERROR] Не удалось выполнить git pull."
    exit 1
fi

if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

if [ ! -d "$PYVENV_DIR" ]; then
    python -m venv venv
fi

source venv/bin/activate
pip install -r requirements.txt

cd "$BUILD_DIR"

echo "Запуск сборки проекта..."
cmake ..
cmake --build .
if [ $? -ne 0 ]; then
    echo "[ERROR] Сборка проекта завершилась с ошибкой."
    exit 1
fi

echo "Процесс завершен успешно, исполняемый файл находится в директории build."
exit 0
