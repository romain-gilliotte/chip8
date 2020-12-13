cmake_minimum_required(VERSION 3.13)

project(chip8 VERSION 1.0.0)

set (CMAKE_CXX_STANDARD 11)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -pg)
endif()


include(CTest)

add_subdirectory(external)
add_subdirectory(libraries/chip8)
add_subdirectory(emulator)
