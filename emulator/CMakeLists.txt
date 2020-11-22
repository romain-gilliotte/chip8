cmake_minimum_required(VERSION 3.13)

project(emulator VERSION 1.0.0)

add_executable(${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    PRIVATE
        src/main.c        
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        SDL2 
        chip8
)
