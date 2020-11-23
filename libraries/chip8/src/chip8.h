#pragma once
#include <inttypes.h>
#include <stdio.h>

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
    uint8_t *display; // one byte per pixel
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

typedef enum Chip8Error
{
    CHIP8_OK,
    CHIP8_ROM_NOT_FOUND,
    CHIP8_ROM_TOO_LONG,
    CHIP8_INVALID_OPCODE,
    CHIP8_CALL_STACK_EMPTY,
    CHIP8_CALL_STACK_FULL,
} Chip8Error;

/**
 * Initialize the Chip8 struct
 */
int chip8_init(Chip8 *state, int width, int height, int clock_speed);

int chip8_load_rom(Chip8 *state, const char *rom);

int chip8_disassemble(FILE *f, uint8_t *program, uint32_t size);

int chip8_dump(FILE *f, Chip8 *state);
int chip8_restore(FILE *f, Chip8 *state);