#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip8.h"

static uint8_t sprites[5 * 16] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

int chip8_init(Chip8 *state, int width, int height, int clock_speed)
{
    memset(state, 0, sizeof *state);

    // Init state
    state->screen_width = width;
    state->screen_height = height;
    state->display = malloc(width * height);
    state->clock_speed = clock_speed;
    state->PC = 0x200;

    // Load fonts
    memcpy(state->memory, sprites, 5 * 16);

    return 0;
}

int chip8_load_rom(Chip8 *state, const char *rom)
{
    // Load ROM
    FILE *f = fopen(rom, "rb");
    if (f == NULL)
        return CHIP8_ROM_NOT_FOUND;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    if (fsize < 0 || sizeof(state->memory) < (unsigned) fsize + 0x200)
        return CHIP8_ROM_TOO_LONG;

    fseek(f, 0, SEEK_SET);
    fread(state->memory + 0x200, fsize, 1, f);
    fclose(f);

    return 0;
}

void chip8_disassemble(Chip8 *state, FILE *f)
{
    uint16_t opcode = state->memory[state->PC + 1] | (state->memory[state->PC] << 8); // big endian
    uint16_t q1 = opcode & 0xF000;
    uint16_t q4 = opcode & 0x000F;

    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t kk = opcode & 0xFF;
    uint16_t nnn = opcode & 0xFFF;

    fprintf(f, "0x%04x: ", state->PC);

    if (q1 == 0x0000)
    {
        if (opcode == 0x00E0)
            fprintf(f, "CLS");
        else if (opcode == 0x00ee)
            fprintf(f, "RET");
        else
            fprintf(f, "Invalid opcode: 0x%04x", opcode);
    }
    else if (q1 == 0x1000)
        fprintf(f, "JMP  0x%04x", nnn);
    else if (q1 == 0x2000)
        fprintf(f, "CALL 0x%04x", nnn);
    else if (q1 == 0x3000)
        fprintf(f, "SE   V%x, 0x%02x", x, kk);
    else if (q1 == 0x4000)
        fprintf(f, "SNE  V%x, 0x%02x", x, kk);
    else if (q1 == 0x5000)
    {
        if ((opcode & 0x000F) == 0)
            fprintf(f, "SE   V%x, V%x", x, y);
        else
            fprintf(f, "Invalid opcode: 0x%04x", opcode);
    }
    else if (q1 == 0x6000)
        fprintf(f, "LD   V%x, 0x%02x", x, kk);
    else if (q1 == 0x7000)
        fprintf(f, "ADD  V%x, 0x%02x", x, kk);
    else if (q1 == 0x8000)
    {
        if (q4 == 0x0000)
            fprintf(f, "LD   V%x, V%x", x, y);
        else if (q4 == 0x0001)
            fprintf(f, "OR   V%x, V%x", x, y);
        else if (q4 == 0x0002)
            fprintf(f, "AND  V%x, V%x", x, y);
        else if (q4 == 0x0003)
            fprintf(f, "XOR  V%x, V%x", x, y);
        else if (q4 == 0x0004)
            fprintf(f, "ADD  V%x, V%x", x, y);
        else if (q4 == 0x0005)
            fprintf(f, "SUB  V%x, V%x", x, y);
        else if (q4 == 0x0006)
            fprintf(f, "SHR  V%x, V%x", x, y);
        else if (q4 == 0x0007)
            fprintf(f, "SUBN V%x, V%x", x, y);
        else if (q4 == 0x000e)
            fprintf(f, "SHL  V%x, V%x", x, y);
        else
            fprintf(f, "Invalid opcode: 0x%04x", opcode);
    }
    else if (q1 == 0x9000)
    {
        if (q4 == 0x0000)
            fprintf(f, "SNE  V%x, V%x", x, y);
        else
            fprintf(f, "Invalid opcode: 0x%04x", opcode);
    }
    else if (q1 == 0xA000)
        fprintf(f, "LD   I,  0x%04x", nnn);
    else if (q1 == 0xB000)
        fprintf(f, "JP   V0, 0x%04x", nnn);
    else if (q1 == 0xC000)
        fprintf(f, "RND  V%x, 0x%02x", x, kk);
    else if (q1 == 0xD000)
        fprintf(f, "DRW  V%x, V%x, %d", x, y, q4);
    else if (q1 == 0xE000)
    {
        if (kk == 0x009e)
            fprintf(f, "SKP  V%x", x);
        else if (kk == 0x00a1)
            fprintf(f, "SKNP V%x", x);
        else
            fprintf(f, "Invalid opcode: 0x%04x", opcode);
    }
    else if (q1 == 0xF000)
    {
        if (kk == 0x07)
            fprintf(f, "LD   V%x, DT", x);
        else if (kk == 0x0a)
            fprintf(f, "LD   V%x, K", x);
        else if (kk == 0x15)
            fprintf(f, "LD   DT, V%x", x);
        else if (kk == 0x18)
            fprintf(f, "LD   ST, V%x", x);
        else if (kk == 0x1e)
            fprintf(f, "ADD  I, V%x", x);
        else if (kk == 0x29)
            fprintf(f, "LD   F, V%x", x);
        else if (kk == 0x33)
            fprintf(f, "LD   B, V%x", x);
        else if (kk == 0x55)
            fprintf(f, "LD   [I], V%x", x);
        else if (kk == 0x65)
            fprintf(f, "LD   V%x, [I]", x);
        else
            fprintf(f, "Invalid opcode: 0x%04x", opcode);
    }

    fprintf(f, "\n");
}

int chip8_dump(Chip8 *state, FILE *f)
{
    (void) state;
    (void) f;

    return 0;
}

int chip8_restore(Chip8 *state, FILE *f)
{
    (void) state;
    (void) f;

    return 0;
}
