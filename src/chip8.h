#pragma once
#include "inttypes.h"

typedef struct Chip8
{
    ////////////
    // Configuration
    ////////////

    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t clock_speed;

    ////////////
    // Emulation state
    ////////////

    uint32_t cycle_counts;

    ////////////
    // Machine state
    ////////////

    uint8_t memory[4096];

    // IO
    uint8_t display[4096]; // one byte per pixel
    uint8_t keyboard[16];

    // Registers
    uint8_t registers[16];
    uint8_t DT;
    uint8_t ST;

    // Pseudo-registers
    uint16_t I;
    uint16_t PC;

    // Stack
    uint8_t SP;
    uint16_t stack[16];
} Chip8;

Chip8 *chip8_create(int width, int height, int clock_speed, const char *rom);
