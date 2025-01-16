cmake_minimum_required(VERSION 3.10)

project(ProcessLibrary)
set(CMAKE_CXX_STANDARD 11)

# Указываем путь к заголовочным файлам
include_directories(${CMAKE_SOURCE_DIR}/Lib)

# Добавляем исходники библиотеки
add_library(ProcessManager STATIC Lib/lib.cpp)

# Добавляем исполняемую программу
add_executable(testProcessManager test.cpp)

# Линкуем тестовую программу с библиотекой
target_link_libraries(testProcessManager ProcessManager)
