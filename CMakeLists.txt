cmake_minimum_required(VERSION 3.10)

project(emulator VERSION 1.0)

find_package(SDL2 REQUIRED)
include_directories(emulator ${SDL2_INCLUDE_DIRS})

add_executable(
    emulator
    src/main.c
    src/chip8.c
    src/interpreter/interpreter.c
)

target_link_libraries(emulator ${SDL2_LIBRARIES})
