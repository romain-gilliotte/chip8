cmake_minimum_required(VERSION 3.13)
project(chip8 VERSION 1.0.0)

####################
# Build library
####################

add_library(${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    PUBLIC
        src/vm.h
    
    PRIVATE
        src/chip8.c
        src/chip8.h
        src/disasm.c
        src/disasm.h
        src/interpreter/interpreter.c
        src/interpreter/interpreter.h
        src/recompiler/recompiler.c
        src/recompiler/recompiler.h
        src/recompiler/translate.c
        src/recompiler/translate.h
        src/recompiler/x64.c
        src/recompiler/x64.h
        src/vm.c
)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    src
)

####################
# Build unit tests
####################

# Create target
add_executable(test-interpreter)
target_sources(
    test-interpreter
    PRIVATE
        test/test-interpreter.c
)

# Link test executable w/ dependencies
target_link_libraries(
    test-interpreter
    PRIVATE chip8
    PRIVATE cmocka
)

add_test(test-chip8 test-interpreter)
