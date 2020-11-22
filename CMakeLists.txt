cmake_minimum_required(VERSION 3.10)

project(chip8 VERSION 1.0.0)

include(CTest)
add_subdirectory(libchip8)
add_subdirectory(emulator)
