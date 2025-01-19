cmake_minimum_required(VERSION 3.10)

project(Lab5)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR})

add_library(sqlite3 OBJECT sqlite3.c)

add_executable(main main.cpp)
add_executable(server server.cpp)
add_executable(simulator simulator.cpp)

if(WIN32)
    target_link_libraries(main ws2_32 sqlite3)
    target_link_libraries(server ws2_32 sqlite3)
else()
    target_link_libraries(main sqlite3)
    target_link_libraries(server sqlite3)
endif()
