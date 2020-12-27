#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "interpreter.h"
#include "../disasm.h"


static Chip8Error exec_not_supported(Chip8 *state, Chip8Opcode* opcode)
{
    (void) state;
    (void) opcode;

    return CHIP8_OPCODE_NOT_SUPPORTED;
}

static Chip8Error exec_invalid(Chip8 *state, Chip8Opcode* opcode)
{
    (void) state;
    (void) opcode;

    return CHIP8_OPCODE_INVALID;
}

/**
 * 00E0 - CLS
 * Clear the display.
 */
static Chip8Error exec_cls(Chip8 *state, Chip8Opcode* opcode)
{
    (void) opcode;

    memset(state->display, 0, state->screen_width * state->screen_height);
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 00EE - RET
 * Return from a subroutine.
 * 
 * The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
 */
static Chip8Error exec_ret(Chip8 *state, Chip8Opcode* opcode)
{
    (void) opcode;

    if (state->SP > 0) {
        state->SP--;
        state->PC = state->stack[state->SP] + 2;
        return CHIP8_OK;
    }
    else {
        return CHIP8_CALL_STACK_EMPTY;
    }
}

/**
 * 1nnn - JP addr
 * Jump to location nnn.
 * 
 * The interpreter sets the program counter to nnn.
 */
static Chip8Error exec_jmp_nnn(Chip8 *state, Chip8Opcode* opcode)
{
    state->PC = opcode->nnn;
    return CHIP8_OK;
}

/**
 * 2nnn - CALL addr
 * Call subroutine at nnn.
 * 
 * The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
 */
static Chip8Error exec_call_nnn(Chip8 *state, Chip8Opcode* opcode)
{
    if (state->SP < 16) {
        state->stack[state->SP] = state->PC;
        state->SP++;
        state->PC = opcode->nnn;
        return CHIP8_OK;
    }
    else {
        return CHIP8_CALL_STACK_FULL;
    }
}

/**
 * 3xkk - SE Vx, byte
 * Skip next instruction if Vx = kk.
 * 
 * The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
 */
static Chip8Error exec_se_vx_kk(Chip8 *state, Chip8Opcode* opcode)
{
    state->PC += state->registers[opcode->x] == opcode->kk ? 4 : 2;
    return CHIP8_OK;
}

/**
 * 4xkk - SNE Vx, byte
 * Skip next instruction if Vx != kk.
 * 
 * The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
 */
static Chip8Error exec_sne_vx_kk(Chip8 *state, Chip8Opcode* opcode)
{
    state->PC += state->registers[opcode->x] != opcode->kk ? 4 : 2;
    return CHIP8_OK;
}

/**
 * 5xy0 - SE Vx, Vy
 * Skip next instruction if Vx = Vy.
 * 
 * The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
 */
static Chip8Error exec_se_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->PC += state->registers[opcode->x] == state->registers[opcode->y] ? 4 : 2;
    return CHIP8_OK;
}

/**
 * 6xkk - LD Vx, byte
 * Set Vx = kk.
 * 
 * The interpreter puts the value kk into register Vx.
 */
static Chip8Error exec_ld_vx_kk(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[opcode->x] = opcode->kk;
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 7xkk - ADD Vx, byte
 * Set Vx = Vx + kk.
 * 
 * Adds the value kk to the value of register Vx, then stores the result in Vx.
 */
static Chip8Error exec_add_vx_kk(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[opcode->x] += opcode->kk;
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 8xy0 - LD Vx, Vy
 * Set Vx = Vy.
 * 
 * Stores the value of register Vy in register Vx.
 */
static Chip8Error exec_ld_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[opcode->x] = state->registers[opcode->y];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 8xy1 - OR Vx, Vy
 * Set Vx = Vx OR Vy.
 * 
 * Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. 
 * A bitwise OR compares the corrseponding bits from two values, and if either bit is 1,
 * then the same bit in the result is also 1. Otherwise, it is 0.
 */
static Chip8Error exec_or_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[opcode->x] |= state->registers[opcode->y];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 8xy2 - AND Vx, Vy
 * Set Vx = Vx AND Vy.
 * 
 * Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
 * A bitwise AND compares the corrseponding bits from two values, and if both bits are 1,
 * then the same bit in the result is also 1. Otherwise, it is 0.
 */
static Chip8Error exec_and_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[opcode->x] &= state->registers[opcode->y];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 8xy3 - XOR Vx, Vy
 * Set Vx = Vx XOR Vy.
 * 
 * Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
 * An exclusive OR compares the corrseponding bits from two values, and if the bits are not 
 * both the same, then the corresponding bit in the result is set to 1. Otherwise, it is 0.
 */
static Chip8Error exec_xor_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[opcode->x] ^= state->registers[opcode->y];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 8xy4 - ADD Vx, Vy
 * Set Vx = Vx + Vy, set VF = carry.
 * 
 * The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,)
 * VF is set to 1, otherwise 0. 
 * Only the lowest 8 bits of the result are kept, and stored in Vx.
 */
static Chip8Error exec_add_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[15] = (uint16_t)state->registers[opcode->x] + (uint16_t)state->registers[opcode->y] > 255;
    state->registers[opcode->x] += state->registers[opcode->y];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 8xy5 - SUB Vx, Vy
 * Set Vx = Vx - Vy, set VF = NOT borrow.
 * 
 * If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
 */
static Chip8Error exec_sub_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[15] = state->registers[opcode->x] > state->registers[opcode->y];
    state->registers[opcode->x] -= state->registers[opcode->y];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 8xy6 - SHR Vx, Vy
 * Set Vx = Vx SHR 1.
 * 
 * If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
 */
static Chip8Error exec_shr_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[15] = state->registers[opcode->x] & 0x1;
    state->registers[opcode->x] >>= 1;
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 8xy7 - SUBN Vx, Vy
 * Set Vx = Vy - Vx, set VF = NOT borrow.
 * 
 * If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
 */
static Chip8Error exec_subn_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[15] = state->registers[opcode->y] > state->registers[opcode->x];
    state->registers[opcode->x] = state->registers[opcode->y] - state->registers[opcode->x];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 8xyE - SHL Vx, Vy
 * Set Vx = Vx SHL 1.
 * 
 * If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
 */
static Chip8Error exec_shl_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[15] = state->registers[opcode->x] >> 7;
    state->registers[opcode->x] <<= 1;
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * 9xy0 - SNE Vx, Vy
 * Skip next instruction if Vx != Vy.
 * 
 * The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
 */
static Chip8Error exec_sne_vx_vy(Chip8 *state, Chip8Opcode* opcode)
{
    state->PC += state->registers[opcode->x] != state->registers[opcode->y] ? 4 : 2;
    return CHIP8_OK;
}

/**
 * Annn - LD I, addr
 * Set I = nnn.
 * 
 * The value of register I is set to nnn.
 */
static Chip8Error exec_ld_i_nnn(Chip8 *state, Chip8Opcode* opcode)
{
    state->I = opcode->nnn;
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Bnnn - JP V0, addr
 * Jump to location nnn + V0.
 * 
 * The program counter is set to nnn plus the value of V0.
 */
static Chip8Error exec_jp_v0_nnn(Chip8 *state, Chip8Opcode* opcode)
{
    state->PC = state->registers[0] + opcode->nnn;
    return CHIP8_OK;
}

/**
 * Cxkk - RND Vx, byte
 * Set Vx = random byte AND kk.
 * 
 * The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.
 * The results are stored in Vx. See instruction 8xy2 for more information on AND.
 */
static Chip8Error exec_rnd_vx_kk(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[opcode->x] = opcode->kk & rand();
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Dxyn - DRW Vx, Vy, nibble
 * Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
 * 
 * The interpreter reads n bytes from memory, starting at the address stored in I.
 * These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
 * Sprites are XORed onto the existing screen. If this causes any pixels to be erased, 
 * VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it
 * is outside the coordinates of the display, it wraps around to the opposite side of 
 * the screen. 
 * 
 * See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more
 * information on the Chip-8 screen and sprites.
 */
static Chip8Error exec_drw_vx_vy_n(Chip8 *state, Chip8Opcode* opcode)
{
    uint8_t x0 = state->registers[opcode->x];
    uint8_t y0 = state->registers[opcode->y];
    uint8_t n = opcode->n;
    
    state->registers[15] = 0;
    for (int y = 0; y < n; ++y)
    {
        for (int x = 0; x < 8; ++x)
        {
            uint16_t position = (y0 + y) % state->screen_height * state->screen_width + (x0 + x) % state->screen_width;
            uint8_t new_pixel = 1 & (state->memory[state->I + y] >> (7 - x));

            state->registers[15] |= new_pixel & state->display[position];
            state->display[position] ^= new_pixel;
        }
    }

    state->display_dirty = true;
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Ex9E - SKP Vx
 * Skip next instruction if key with the value of Vx is pressed.
 * 
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
 */
static Chip8Error exec_skp_vx(Chip8 *state, Chip8Opcode* opcode)
{
    state->PC += state->keyboard[state->registers[opcode->x]] ? 4 : 2;
    return CHIP8_OK;
}

/**
 * ExA1 - SKNP Vx
 * Skip next instruction if key with the value of Vx is not pressed.
 * 
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
 */
static Chip8Error exec_sknp_vx(Chip8 *state, Chip8Opcode* opcode)
{
    state->PC += state->keyboard[state->registers[opcode->x]] ? 2 : 4;
    return CHIP8_OK;
}

/**
 * Fx07 - LD Vx, DT
 * Set Vx = delay timer value.
 * 
 * The value of DT is placed into Vx.
 */
static Chip8Error exec_ld_vx_dt(Chip8 *state, Chip8Opcode* opcode)
{
    state->registers[opcode->x] = state->DT;
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Fx0A - LD Vx, K
 * Wait for a key press, store the value of the key in Vx.
 * 
 * All execution stops until a key is pressed, then the value of that key is stored in Vx.
 */
static Chip8Error exec_ld_vx_k(Chip8 *state, Chip8Opcode* opcode)
{
    int i;
    for (i = 0; i < 16; ++i)
        if (state->keyboard[i])
        {
            state->registers[opcode->x] = i;
            break;
        }

    if (i != 16)
        state->PC += 2;

    return CHIP8_OK;
}

/**
 * Fx15 - LD DT, Vx
 * Set delay timer = Vx.
 * 
 * DT is set equal to the value of Vx.
 */
static Chip8Error exec_ld_dt_vx(Chip8 *state, Chip8Opcode* opcode)
{
    state->DT = state->registers[opcode->x];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Fx18 - LD ST, Vx
 * Set sound timer = Vx.
 * 
 * ST is set equal to the value of Vx.
 */
static Chip8Error exec_ld_st_vx(Chip8 *state, Chip8Opcode* opcode)
{
    state->ST = state->registers[opcode->x];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Fx1E - ADD I, Vx
 * Set I = I + Vx.
 * 
 * The values of I and Vx are added, and the results are stored in I.
 */
static Chip8Error exec_add_i_vx(Chip8 *state, Chip8Opcode* opcode)
{
    state->I += state->registers[opcode->x];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Fx29 - LD F, Vx
 * Set I = location of sprite for digit Vx.
 * 
 * The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
 * See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
 */
static Chip8Error exec_ld_f_vx(Chip8 *state, Chip8Opcode* opcode)
{
    state->I = 5 * state->registers[opcode->x];
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Fx33 - LD B, Vx
 * Store BCD representation of Vx in memory locations I, I+1, and I+2.
 * 
 * The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I,
 * the tens digit at location I+1, and the ones digit at location I+2.
 */
static Chip8Error exec_ld_b_vx(Chip8 *state, Chip8Opcode* opcode)
{
    uint8_t remainder = state->registers[opcode->x];

    for (int i = 0; i < 3; ++i)
    {
        state->memory[2 + state->I - i] = remainder % 10;
        remainder = remainder / 10;
    }

    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Fx55 - LD [I], Vx
 * Store registers V0 through Vx in memory starting at location I.
 * 
 * The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
 */
static Chip8Error exec_ld_i_vx(Chip8 *state, Chip8Opcode* opcode)
{
    for (int i = 0; i <= opcode->x; ++i)
        state->memory[state->I + i] = state->registers[i];

    state->I += opcode->x + 1;
    state->PC += 2;
    return CHIP8_OK;
}

/**
 * Fx65 - LD Vx, [I]
 * Read registers V0 through Vx from memory starting at location I.
 * 
 * The interpreter reads values from memory starting at location I into registers V0 through Vx.
 */
static Chip8Error exec_ld_vx_i(Chip8 *state, Chip8Opcode* opcode)
{
    for (int i = 0; i <= opcode->x; ++i)
        state->registers[i] = state->memory[state->I + i];

    state->I += opcode->x + 1;
    state->PC += 2;
    return CHIP8_OK;
}


static Chip8Error (*exec_opcode[])(Chip8 *, Chip8Opcode*) = {
    // Original
    exec_invalid,       // OPCODE_INVALID
    exec_cls,           // OPCODE_CLS,
    exec_ret,           // OPCODE_RET,
    exec_jmp_nnn,       // OPCODE_JMP_NNN,
    exec_call_nnn,      // OPCODE_CALL_NNN,
    exec_se_vx_kk,      // OPCODE_SE_VX_KK,
    exec_sne_vx_kk,     // OPCODE_SNE_VX_KK,
    exec_se_vx_vy,      // OPCODE_SE_VX_VY,
    exec_ld_vx_kk,      // OPCODE_LD_VX_KK,
    exec_add_vx_kk,     // OPCODE_ADD_VX_KK,
    exec_ld_vx_vy,      // OPCODE_LD_VX_VY,
    exec_or_vx_vy,      // OPCODE_OR_VX_VY,
    exec_and_vx_vy,     // OPCODE_AND_VX_VY,
    exec_xor_vx_vy,     // OPCODE_XOR_VX_VY,
    exec_add_vx_vy,     // OPCODE_ADD_VX_VY,
    exec_sub_vx_vy,     // OPCODE_SUB_VX_VY,
    exec_shr_vx_vy,     // OPCODE_SHR_VX_VY,
    exec_subn_vx_vy,    // OPCODE_SUBN_VX_VY,
    exec_shl_vx_vy,     // OPCODE_SHL_VX_VY,
    exec_sne_vx_vy,     // OPCODE_SNE_VX_VY,
    exec_ld_i_nnn,      // OPCODE_LD_I_NNN,
    exec_jp_v0_nnn,     // OPCODE_JP_V0_NNN,
    exec_rnd_vx_kk,     // OPCODE_RND_VX_KK,
    exec_drw_vx_vy_n,   // OPCODE_DRW_VX_VY_N,
    exec_skp_vx,        // OPCODE_SKP_VX,
    exec_sknp_vx,       // OPCODE_SKNP_VX,
    exec_ld_vx_dt,      // OPCODE_LD_VX_DT,
    exec_ld_vx_k,       // OPCODE_LD_VX_K,
    exec_ld_dt_vx,      // OPCODE_LD_DT_VX,
    exec_ld_st_vx,      // OPCODE_LD_ST_VX,
    exec_add_i_vx,      // OPCODE_ADD_I_VX,
    exec_ld_f_vx,       // OPCODE_LD_F_VX,
    exec_ld_b_vx,       // OPCODE_LD_B_VX,
    exec_ld_i_vx,       // OPCODE_LD_I_VX,
    exec_ld_vx_i,       // OPCODE_LD_VX_I,
    
    // S-Chip
    exec_not_supported, // OPCODE_SCRL_DOWN_N,
    exec_not_supported, // OPCODE_SCRL_LEFT,
    exec_not_supported, // OPCODE_SCRL_RIGHT,
    exec_not_supported, // OPCODE_EXIT,
    exec_not_supported, // OPCODE_HIDEF_OFF,
    exec_not_supported, // OPCODE_HIDEF_ON,
    exec_not_supported, // OPCODE_LD_I_,
    exec_not_supported, // OPCODE_LD_RPL_VX,
    exec_not_supported, // OPCODE_LD_VX_RPL,

    // XO-Chip
    exec_not_supported, // OPCODE_LD_I_VX_VY,
    exec_not_supported, // OPCODE_LD_VX_VY_I,
    exec_not_supported, // OPCODE_LD_I_NNNN,
    exec_not_supported, // OPCODE_DRW_PLN_N,
    exec_not_supported, // OPCODE_LD_AUDIO_I,
    exec_not_supported, // OPCODE_SCRL_UP_N,
};


Chip8Error interpreter_step(Chip8 *state)
{
    chip8_disassemble(state, stdout);
    Chip8Opcode opcode;
    chip8_decode(state, &opcode, state->PC);
   
    Chip8Error error = exec_opcode[opcode.id](state, &opcode);
    if (error != CHIP8_OK)
        return error;

    state->cycles_since_started++;
    return CHIP8_OK;
}

