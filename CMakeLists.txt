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


# add_executable(test-chip8 test/chip8.c)

# target_link_libraries(test-chip8 PRIVATE libchip8)

# /usr/include/cmocka.h
# /usr/include/cmocka_pbc.h
# /usr/lib/x86_64-linux-gnu/libcmocka.so
# /usr/lib/x86_64-linux-gnu/libcmocka.so.0
# /usr/lib/x86_64-linux-gnu/libcmocka.so.0.7.0
