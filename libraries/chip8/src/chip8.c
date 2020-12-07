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
    if (fsize + 0x200 > sizeof(state->memory))
        return CHIP8_ROM_TOO_LONG;

    fseek(f, 0, SEEK_SET);
    fread(state->memory + 0x200, fsize, 1, f);
    fclose(f);

    return 0;
}

int chip8_disassemble(Chip8 *state, FILE *f)
{
    uint16_t opcode = state->memory[state->PC + 1] | (state->memory[state->PC] << 8); // big endian
    uint16_t q1 = opcode & 0xF000;
    uint16_t q4 = opcode & 0x000F;

    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t kk = opcode & 0xFF;
    uint16_t nnn = opcode & 0xFFF;

    if (q1 == 0x0000)
    {
        if (opcode == 0x00E0)
            fprintf(f, "0x%04x: CLS  # Clear the display\n", state->PC);
        else if (opcode == 0x00ee)
            fprintf(f, "0x%04x: RET  # Return from a subroutine\n", state->PC);
        else
            fprintf(f, "0x%04x: Invalid opcode: 0x%04x\n", state->PC, opcode);
    }
    else if (q1 == 0x1000)
        fprintf(f, "0x%04x: JMP  0x%04x\n", state->PC, nnn);
    else if (q1 == 0x2000)
        fprintf(f, "0x%04x: CALL 0x%04x\n", state->PC, nnn);
    else if (q1 == 0x3000)
        fprintf(f, "0x%04x: SE   V%x, 0x%02x\n", state->PC, x, kk);
    else if (q1 == 0x4000)
        fprintf(f, "0x%04x: SNE  V%x, 0x%02x\n", state->PC, x, kk);
    else if (q1 == 0x5000)
    {
        if ((opcode & 0x000F) == 0)
            fprintf(f, "0x%04x: SE   V%x, V%x\n", state->PC, x, y);
        else
            fprintf(f, "0x%04x: Invalid opcode: 0x%04x\n", state->PC, opcode);
    }
    else if (q1 == 0x6000)
        fprintf(f, "0x%04x: LD   V%x, 0x%02x\n", state->PC, x, kk);
    else if (q1 == 0x7000)
        fprintf(f, "0x%04x: ADD  V%x, 0x%02x\n", state->PC, x, kk);
    else if (q1 == 0x8000)
    {
        if (q4 == 0x0000)
            fprintf(f, "0x%04x: LD   V%x, V%x\n", state->PC, x, y);
        else if (q4 == 0x0001)
            fprintf(f, "0x%04x: OR   V%x, V%x\n", state->PC, x, y);
        else if (q4 == 0x0002)
            fprintf(f, "0x%04x: AND   V%x, V%x\n", state->PC, x, y);
        else if (q4 == 0x0003)
            fprintf(f, "0x%04x: XOR  V%x, V%x\n", state->PC, x, y);
        else if (q4 == 0x0004)
            fprintf(f, "0x%04x: ADD  V%x, V%x\n", state->PC, x, y);
        else if (q4 == 0x0005)
            fprintf(f, "0x%04x: SUB  V%x, V%x\n", state->PC, x, y);
        else if (q4 == 0x0006)
            fprintf(f, "0x%04x: SHR  V%x, V%x\n", state->PC, x, y);
        else if (q4 == 0x0007)
            fprintf(f, "0x%04x: SUBN V%x, V%x\n", state->PC, x, y);
        else if (q4 == 0x000e)
            fprintf(f, "0x%04x: SHL  V%x, V%x\n", state->PC, x, y);
        else
            fprintf(f, "0x%04x: Invalid opcode: 0x%04x\n", state->PC, opcode);
    }
    else if (q1 == 0x9000)
    {
        if (q4 == 0x0000)
            fprintf(f, "0x%04x: SNE  V%x, V%x\n", state->PC, x, y);
        else
            fprintf(f, "0x%04x: Invalid opcode: 0x%04x\n", state->PC, opcode);
    }
    else if (q1 == 0xA000)
        fprintf(f, "0x%04x: LD   I,  0x%04x\n", state->PC, nnn);
    else if (q1 == 0xB000)
        fprintf(f, "0x%04x: JP   V0, 0x%04x\n", state->PC, nnn);
    else if (q1 == 0xC000)
        fprintf(f, "0x%04x: RND  V%x, 0x%02x\n", state->PC, x, kk);
    else if (q1 == 0xD000)
        fprintf(f, "0x%04x: DRW  V%x, V%x, %d\n", state->PC, x, y, q4);
    else if (q1 == 0xE000)
    {
        if (kk == 0x009e)
            fprintf(f, "0x%04x: SKP  V%x\n", state->PC, x);
        else if (kk == 0x00a1)
            fprintf(f, "0x%04x: SKNP V%x\n", state->PC, x);
        else
            fprintf(f, "0x%04x: Invalid opcode: 0x%04x\n", state->PC, opcode);
    }
    else if (q1 == 0xF000)
    {
        if (kk == 0x07)
            fprintf(f, "0x%04x: LD   V%x, DT\n", state->PC, x);
        else if (kk == 0x0a)
            fprintf(f, "0x%04x: LD   V%x, K\n", state->PC, x);
        else if (kk == 0x15)
            fprintf(f, "0x%04x: LD   DT, V%x\n", state->PC, x);
        else if (kk == 0x18)
            fprintf(f, "0x%04x: LD   ST, V%x\n", state->PC, x);
        else if (kk == 0x1e)
            fprintf(f, "0x%04x: ADD  I, V%x\n", state->PC, x);
        else if (kk == 0x29)
            fprintf(f, "0x%04x: LD   F, V%x\n", state->PC, x);
        else if (kk == 0x33)
            fprintf(f, "0x%04x: LD   B, V%x\n", state->PC, x);
        else if (kk == 0x55)
            fprintf(f, "0x%04x: LD   [I], V%x\n", state->PC, x);
        else if (kk == 0x65)
            fprintf(f, "0x%04x: LD   V%x, [I]\n", state->PC, x);
        else
            fprintf(f, "0x%04x: Invalid opcode: 0x%04x\n", state->PC, opcode);
    }
}

int chip8_dump(Chip8 *state, FILE *f)
{
    return 0;
}

int chip8_restore(Chip8 *state, FILE *f)
{
    return 0;
}
