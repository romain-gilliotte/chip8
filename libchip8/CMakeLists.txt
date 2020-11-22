cmake_minimum_required(VERSION 3.10)
project(chip8 VERSION 1.0.0)

####################
# Build library
####################

# Create target
add_library(
    ${PROJECT_NAME}
    src/chip8.c
    src/interpreter/interpreter.c
)

# Tell where the headers are for target which want to link with us.
target_include_directories(
    ${PROJECT_NAME}
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

####################
# Build unit tests
####################

find_package(cmocka REQUIRED)

# Create target
add_executable(test-interpreter test/chip8.c)

# Add include directories w/ dependencies
target_include_directories(
    test-interpreter
    PRIVATE ${CMOCKA_INCLUDE_DIR}
)

# Link test executable w/ dependencies
target_link_libraries(
    test-interpreter
    PRIVATE chip8
    PRIVATE ${CMOCKA_LIBRARIES}
)

add_test(test-chip8 test-interpreter)
