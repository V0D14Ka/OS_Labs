cmake_minimum_required(VERSION 3.10)

project(task4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp)
add_executable(${PROJECT_NAME} simulator.cpp)

# if(UNIX)
#     target_link_libraries(${PROJECT_NAME} pthread)
# endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
