cmake_minimum_required(VERSION 3.13)

project(chip8 VERSION 1.0.0)

include(CTest)

add_subdirectory(external)
add_subdirectory(libraries/chip8)
add_subdirectory(emulator)
