#!/bin/bash

# Определяем директорию проекта
PROJECT_DIR=$(dirname "$(realpath "$0")")

echo "Переход в директорию проекта: $PROJECT_DIR"
cd "$PROJECT_DIR" || { echo "[ERROR] Невозможно перейти в директорию $PROJECT_DIR."; exit 1; }

echo "Обновление репозитория..."
if ! git pull; then
    echo "[ERROR] Не удалось выполнить git pull."
    exit 1
fi

echo "Запуск сборки проекта..."
if ! make; then
    echo "[ERROR] Сборка проекта завершилась с ошибкой."
    exit 1
fi

echo "Процесс завершен успешно."
exit 0
