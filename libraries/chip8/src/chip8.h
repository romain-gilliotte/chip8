#pragma once
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct
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

    uint32_t cycles_since_started;
    bool display_dirty;

    ////////////
    // Machine
    ////////////

    uint8_t memory[4096];

    // IO
    bool *display; // one byte per pixel
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

typedef enum
{
    CHIP8_OK = 0,
    CHIP8_ROM_NOT_FOUND = -1,
    CHIP8_ROM_TOO_LONG = -2,
    CHIP8_OPCODE_INVALID = -3,
    CHIP8_OPCODE_NOT_SUPPORTED = -4,
    CHIP8_CALL_STACK_EMPTY = -5,
    CHIP8_CALL_STACK_FULL = -6,
} Chip8Error;

/**
 * Initialize the Chip8 struct
 * 
 * @param state Pointer to a newly allocated Chip8.
 * @param width Desired width of the screen. 64 for the original chip8.
 * @param height Desired height of the screen. 48 for the original chip8.
 * @param clock_speed Clock speed in Hz. 500 for the original chip8.
 * @returns 
 */
int chip8_init(Chip8 *state, int width, int height, int clock_speed);
int chip8_load_rom(Chip8 *state, const char *rom);
void chip8_disassemble(Chip8 *state, FILE *f);
int chip8_dump(Chip8 *state, FILE *f);
int chip8_restore(Chip8 *state, FILE *f);
