cmake_minimum_required(VERSION 3.10)

project(emulator VERSION 1.0.0)

find_package(SDL2 REQUIRED)

add_executable(
    ${PROJECT_NAME}
    src/main.c
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${SDL2_INCLUDE_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ${SDL2_LIBRARIES}
    PRIVATE chip8
)
