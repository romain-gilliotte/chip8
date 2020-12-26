#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip8.h"

static uint32_t memory_size[] = {4096, 4096, 4096, 65536};
static uint32_t screen_width[] = {64, 64, 128, 64};
static uint32_t screen_height[] = {32, 64, 128, 64};
static uint32_t screen_channels[] = {1, 1, 1, 2};

static uint8_t sprites[] = {
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

Chip8Error chip8_init(Chip8 *state, Chip8Variant variant, uint32_t clock_speed)
{
    memset(state, 0, sizeof *state);
    state->variant = variant;
    state->clock_speed = clock_speed;
    state->memory = (uint8_t*) malloc(memory_size[variant]);

    if (variant == VARIANT_CHIP8_2PAGE) state->PC = 0x02c0;
    else state->PC = 0x0200;

    state->screen_width = screen_width[variant];
    state->screen_height = screen_height[variant];
    state->display = (bool*) malloc(screen_width[variant] * screen_height[variant] * screen_channels[variant]);
    memcpy(state->memory, sprites, 5 * 16); // Fonts

    return CHIP8_OK;
}

Chip8Error chip8_load_rom(Chip8 *state, const char *rom)
{
    FILE *f = fopen(rom, "rb");
    if (f == NULL)
        return CHIP8_ROM_NOT_FOUND;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    if (fsize < 0 || memory_size[state->variant] < (uint32_t) fsize + 0x200)
        return CHIP8_ROM_TOO_LONG;

    fseek(f, 0, SEEK_SET);
    fread(state->memory + 0x200, fsize, 1, f);
    fclose(f);

    return CHIP8_OK;
}

Chip8Error chip8_decode(Chip8 *state, Chip8Opcode* decoded, uint16_t address) {
    uint16_t opcode = decoded->opcode = state->memory[address + 1] | (state->memory[address] << 8); // big endian
    uint16_t n1 = opcode & 0xF000;
    uint16_t n4 = opcode & 0x000F;
    uint8_t kk = decoded->kk = opcode & 0xFF;
    
    decoded->x = (opcode >> 8) & 0x0F;
    decoded->y = (opcode >> 4) & 0x0F;
    decoded->n = opcode & 0x0F;
    decoded->nnn = opcode & 0x0FFF;

    if (n1 == 0x0000)
    {
        if (opcode == 0x00E0) decoded->id = OPCODE_CLS;
        else if (opcode == 0x00ee) decoded->id = OPCODE_RET;
        else if (state->variant == VARIANT_CHIP8_2PAGE && opcode == 0x230) decoded->id = OPCODE_CLS_HIRES;
        else decoded->id = OPCODE_INVALID;
    }
    else if (n1 == 0x1000) decoded->id = OPCODE_JMP_NNN;
    else if (n1 == 0x2000) decoded->id = OPCODE_CALL_NNN;
    else if (n1 == 0x3000) decoded->id = OPCODE_SE_VX_KK;
    else if (n1 == 0x4000) decoded->id = OPCODE_SNE_VX_KK;
    else if (n1 == 0x5000)
    {
        if ((opcode & 0x000F) == 0) decoded->id = OPCODE_SE_VX_VY;
        else decoded->id = OPCODE_INVALID;
    }
    else if (n1 == 0x6000) decoded->id = OPCODE_LD_VX_KK;
    else if (n1 == 0x7000) decoded->id = OPCODE_ADD_VX_KK;
    else if (n1 == 0x8000)
    {
        if (n4 == 0x0000) decoded->id = OPCODE_LD_VX_VY;
        else if (n4 == 0x0001) decoded->id = OPCODE_OR_VX_VY;
        else if (n4 == 0x0002) decoded->id = OPCODE_AND_VX_VY;
        else if (n4 == 0x0003) decoded->id = OPCODE_XOR_VX_VY;
        else if (n4 == 0x0004) decoded->id = OPCODE_ADD_VX_VY;
        else if (n4 == 0x0005) decoded->id = OPCODE_SUB_VX_VY;
        else if (n4 == 0x0006) decoded->id = OPCODE_SHR_VX_VY;
        else if (n4 == 0x0007) decoded->id = OPCODE_SUBN_VX_VY;
        else if (n4 == 0x000e) decoded->id = OPCODE_SHL_VX_VY;
        else decoded->id = OPCODE_INVALID;
    }
    else if (n1 == 0x9000)
    {
        if (n4 == 0x0000) decoded->id = OPCODE_SNE_VX_VY;
        else decoded->id = OPCODE_INVALID;
    }
    else if (n1 == 0xA000) decoded->id = OPCODE_LD_I_NNN;
    else if (n1 == 0xB000) decoded->id = OPCODE_JP_V0_NNN;
    else if (n1 == 0xC000) decoded->id = OPCODE_RND_VX_KK;
    else if (n1 == 0xD000) decoded->id = OPCODE_DRW_VX_VY_N;
    else if (n1 == 0xE000)
    {
        if (kk == 0x009e) decoded->id = OPCODE_SKP_VX;
        else if (kk == 0x00a1) decoded->id = OPCODE_SKNP_VX;
        else decoded->id = OPCODE_INVALID;
    }
    else if (n1 == 0xF000)
    {
        if (kk == 0x07) decoded->id = OPCODE_LD_VX_DT;
        else if (kk == 0x0a) decoded->id = OPCODE_LD_VX_K;
        else if (kk == 0x15) decoded->id = OPCODE_LD_DT_VX;
        else if (kk == 0x18) decoded->id = OPCODE_LD_ST_VX;
        else if (kk == 0x1e) decoded->id = OPCODE_ADD_I_VX;
        else if (kk == 0x29) decoded->id = OPCODE_LD_F_VX;
        else if (kk == 0x33) decoded->id = OPCODE_LD_B_VX;
        else if (kk == 0x55) decoded->id = OPCODE_LD_I_VX;
        else if (kk == 0x65) decoded->id = OPCODE_LD_VX_I;
        else decoded->id = OPCODE_INVALID;
    }

    return decoded->id == OPCODE_INVALID ? CHIP8_OPCODE_INVALID : CHIP8_OK;
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
