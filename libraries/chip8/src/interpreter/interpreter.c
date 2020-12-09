#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "interpreter.h"

/**
 * 00E0 - CLS
 * Clear the display.
 */
static void exec_00e0(Chip8 *state)
{
    memset(state->display, 0, state->screen_width * state->screen_height);
    state->PC += 2;
}

/**
 * 00EE - RET
 * Return from a subroutine.
 * 
 * The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
 */
static void exec_00ee(Chip8 *state)
{
    state->SP--;
    state->PC = state->stack[state->SP] + 2;
}

/**
 * 1nnn - JP addr
 * Jump to location nnn.
 * 
 * The interpreter sets the program counter to nnn.
 */
static void exec_1nnn(Chip8 *state)
{
    uint16_t addr = ((uint16_t)(state->memory[state->PC] & 0x0F) << 8) + state->memory[state->PC + 1];

    state->PC = addr;
}

/**
 * 2nnn - CALL addr
 * Call subroutine at nnn.
 * 
 * The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
 */
static void exec_2nnn(Chip8 *state)
{
    uint16_t addr = ((uint16_t)(state->memory[state->PC] & 0x0F) << 8) + state->memory[state->PC + 1];

    state->stack[state->SP] = state->PC;
    state->SP++;
    state->PC = addr;
}

/**
 * 3xkk - SE Vx, byte
 * Skip next instruction if Vx = kk.
 * 
 * The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
 */
static void exec_3xkk(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t cst = state->memory[state->PC + 1];

    state->PC += state->registers[x] == cst ? 4 : 2;
}

/**
 * 4xkk - SNE Vx, byte
 * Skip next instruction if Vx != kk.
 * 
 * The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
 */
static void exec_4xkk(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t cst = state->memory[state->PC + 1];

    state->PC += state->registers[x] != cst ? 4 : 2;
}

/**
 * 5xy0 - SE Vx, Vy
 * Skip next instruction if Vx = Vy.
 * 
 * The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
 */
static void exec_5xy0(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->PC += state->registers[x] == state->registers[y] ? 4 : 2;
}

/**
 * 6xkk - LD Vx, byte
 * Set Vx = kk.
 * 
 * The interpreter puts the value kk into register Vx.
 */
static void exec_6xkk(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t cst = state->memory[state->PC + 1];

    state->registers[x] = cst;
    state->PC += 2;
}

/**
 * 7xkk - ADD Vx, byte
 * Set Vx = Vx + kk.
 * 
 * Adds the value kk to the value of register Vx, then stores the result in Vx.
 */
static void exec_7xkk(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t cst = state->memory[state->PC + 1];

    state->registers[x] += cst;
    state->PC += 2;
}

/**
 * 8xy0 - LD Vx, Vy
 * Set Vx = Vy.
 * 
 * Stores the value of register Vy in register Vx.
 */
static void exec_8xy0(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->registers[x] = state->registers[y];
    state->PC += 2;
}

/**
 * 8xy1 - OR Vx, Vy
 * Set Vx = Vx OR Vy.
 * 
 * Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. 
 * A bitwise OR compares the corrseponding bits from two values, and if either bit is 1,
 * then the same bit in the result is also 1. Otherwise, it is 0.
 */
static void exec_8xy1(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->registers[x] |= state->registers[y];
    state->PC += 2;
}

/**
 * 8xy2 - AND Vx, Vy
 * Set Vx = Vx AND Vy.
 * 
 * Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
 * A bitwise AND compares the corrseponding bits from two values, and if both bits are 1,
 * then the same bit in the result is also 1. Otherwise, it is 0.
 */
static void exec_8xy2(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->registers[x] &= state->registers[y];
    state->PC += 2;
}

/**
 * 8xy3 - XOR Vx, Vy
 * Set Vx = Vx XOR Vy.
 * 
 * Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
 * An exclusive OR compares the corrseponding bits from two values, and if the bits are not 
 * both the same, then the corresponding bit in the result is set to 1. Otherwise, it is 0.
 */
static void exec_8xy3(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->registers[x] ^= state->registers[y];
    state->PC += 2;
}

/**
 * 8xy4 - ADD Vx, Vy
 * Set Vx = Vx + Vy, set VF = carry.
 * 
 * The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,)
 * VF is set to 1, otherwise 0. 
 * Only the lowest 8 bits of the result are kept, and stored in Vx.
 */
static void exec_8xy4(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->registers[15] = (uint16_t)state->registers[x] + (uint16_t)state->registers[y] > 255;
    state->registers[x] += state->registers[y];
    state->PC += 2;
}

/**
 * 8xy5 - SUB Vx, Vy
 * Set Vx = Vx - Vy, set VF = NOT borrow.
 * 
 * If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
 */
static void exec_8xy5(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->registers[15] = state->registers[x] > state->registers[y];
    state->registers[x] -= state->registers[y];
    state->PC += 2;
}

/**
 * 8xy6 - SHR Vx, Vy
 * Set Vx = Vx SHR 1.
 * 
 * If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
 */
static void exec_8xy6(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->registers[15] = state->registers[x] & 0x1;
    state->registers[x] >>= 1;
    state->PC += 2;
}

/**
 * 8xy7 - SUBN Vx, Vy
 * Set Vx = Vy - Vx, set VF = NOT borrow.
 * 
 * If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
 */
static void exec_8xy7(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->registers[15] = state->registers[y] > state->registers[x];
    state->registers[x] = state->registers[y] - state->registers[x];
    state->PC += 2;
}

/**
 * 8xyE - SHL Vx, Vy
 * Set Vx = Vx SHL 1.
 * 
 * If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
 */
static void exec_8xye(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->registers[15] = state->registers[x] >> 7;
    state->registers[x] <<= 1;
    state->PC += 2;
}

/**
 * 9xy0 - SNE Vx, Vy
 * Skip next instruction if Vx != Vy.
 * 
 * The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
 */
static void exec_9xy0(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t y = state->memory[state->PC + 1] >> 4;

    state->PC += state->registers[x] != state->registers[y] ? 4 : 2;
}

/**
 * Annn - LD I, addr
 * Set I = nnn.
 * 
 * The value of register I is set to nnn.
 */
static void exec_annn(Chip8 *state)
{
    uint16_t addr = ((uint16_t)(state->memory[state->PC] & 0x0F) << 8) + state->memory[state->PC + 1];

    state->I = addr;
    state->PC += 2;
}

/**
 * Bnnn - JP V0, addr
 * Jump to location nnn + V0.
 * 
 * The program counter is set to nnn plus the value of V0.
 */
static void exec_bnnn(Chip8 *state)
{
    uint16_t addr = ((uint16_t)(state->memory[state->PC] & 0x0F) << 8) + state->memory[state->PC + 1];

    state->PC = state->registers[0] + addr;
}

/**
 * Cxkk - RND Vx, byte
 * Set Vx = random byte AND kk.
 * 
 * The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.
 * The results are stored in Vx. See instruction 8xy2 for more information on AND.
 */
static void exec_cxkk(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    state->registers[x] = state->memory[state->PC + 1] & rand();
    state->PC += 2;
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
static void exec_dxyn(Chip8 *state)
{
    uint8_t x0 = state->registers[state->memory[state->PC] & 0x0F];
    uint8_t y0 = state->registers[(state->memory[state->PC + 1] >> 4) & 0x0F];
    uint8_t n = state->memory[state->PC + 1] & 0x0F;

    state->registers[15] = 0;
    for (int y = 0; y < n; ++y)
    {
        for (int x = 0; x < 8; ++x)
        {
            uint16_t position = (y0 + y) * state->screen_width + x0 + x;
            uint8_t new_pixel = 1 & (state->memory[state->I + y] >> (7 - x));

            state->registers[15] |= new_pixel & state->display[position];
            state->display[position] ^= new_pixel;
        }
    }

    state->display_dirty = true;
    state->PC += 2;
}

/**
 * Ex9E - SKP Vx
 * Skip next instruction if key with the value of Vx is pressed.
 * 
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
 */
static void exec_ex9e(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    state->PC += state->keyboard[state->registers[x]] ? 4 : 2;
}

/**
 * ExA1 - SKNP Vx
 * Skip next instruction if key with the value of Vx is not pressed.
 * 
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
 */
static void exec_exa1(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    state->PC += state->keyboard[state->registers[x]] ? 2 : 4;
}

/**
 * Fx07 - LD Vx, DT
 * Set Vx = delay timer value.
 * 
 * The value of DT is placed into Vx.
 */
static void exec_fx07(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    state->registers[x] = state->DT;
    state->PC += 2;
}

/**
 * Fx0A - LD Vx, K
 * Wait for a key press, store the value of the key in Vx.
 * 
 * All execution stops until a key is pressed, then the value of that key is stored in Vx.
 */
static void exec_fx0a(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    int i;
    for (i = 0; i < 16; ++i)
        if (state->keyboard[i])
        {
            state->registers[x] = i;
            break;
        }

    if (i != 16)
        state->PC += 2;
}

/**
 * Fx15 - LD DT, Vx
 * Set delay timer = Vx.
 * 
 * DT is set equal to the value of Vx.
 */
static void exec_fx15(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    state->DT = state->registers[x];
    state->PC += 2;
}

/**
 * Fx18 - LD ST, Vx
 * Set sound timer = Vx.
 * 
 * ST is set equal to the value of Vx.
 */
static void exec_fx18(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    state->ST = state->registers[x];
    state->PC += 2;
}

/**
 * Fx1E - ADD I, Vx
 * Set I = I + Vx.
 * 
 * The values of I and Vx are added, and the results are stored in I.
 */
static void exec_fx1e(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    state->I += state->registers[x];
    state->PC += 2;
}

/**
 * Fx29 - LD F, Vx
 * Set I = location of sprite for digit Vx.
 * 
 * The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
 * See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
 */
static void exec_fx29(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    state->I = 5 * state->registers[x];
    state->PC += 2;
}

/**
 * Fx33 - LD B, Vx
 * Store BCD representation of Vx in memory locations I, I+1, and I+2.
 * 
 * The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I,
 * the tens digit at location I+1, and the ones digit at location I+2.
 */
static void exec_fx33(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;
    uint8_t remainder = state->registers[x];

    for (int i = 0; i < 3; ++i)
    {
        state->memory[2 + state->I - i] = remainder % 10;
        remainder = remainder / 10;
    }

    state->PC += 2;
}

/**
 * Fx55 - LD [I], Vx
 * Store registers V0 through Vx in memory starting at location I.
 * 
 * The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
 */
static void exec_fx55(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    for (int i = 0; i <= x; ++i)
        state->memory[state->I + i] = state->registers[i];

    state->I += x + 1;
    state->PC += 2;
}

/**
 * Fx65 - LD Vx, [I]
 * Read registers V0 through Vx from memory starting at location I.
 * 
 * The interpreter reads values from memory starting at location I into registers V0 through Vx.
 */
static void exec_fx65(Chip8 *state)
{
    uint8_t x = state->memory[state->PC] & 0x0F;

    for (int i = 0; i <= x; ++i)
        state->registers[i] = state->memory[state->I + i];

    state->I += x + 1;
    state->PC += 2;
}

Chip8Error interpreter_run(Chip8 *state, uint32_t ticks)
{
    uint64_t expected_cc = ticks * state->clock_speed / 1000;

    Chip8Error result = CHIP8_OK;
    while (!result && state->cycle_counts < expected_cc)
    {
        result = interpreter_step(state);
    }

    return result;
}

Chip8Error interpreter_step(Chip8 *state)
{
    chip8_disassemble(state, stdout);

    // Run current opcode.
    uint16_t opcode = ((uint16_t)state->memory[state->PC] << 8) | state->memory[state->PC + 1];

    switch (opcode & 0xF000)
    {
    case 0x0000:
        switch (opcode)
        {
        case 0x00e0:
            exec_00e0(state);
            break;

        case 0x00ee:
            exec_00ee(state);
            break;
        default:
            return CHIP8_OPCODE_INVALID;
        }
        break;

    case 0x1000:
        exec_1nnn(state);
        break;
    case 0x2000:
        exec_2nnn(state);
        break;
    case 0x3000:
        exec_3xkk(state);
        break;
    case 0x4000:
        exec_4xkk(state);
        break;
    case 0x5000:

        if ((opcode & 0x000F) == 0)
            exec_5xy0(state);
        else
            return CHIP8_OPCODE_INVALID;
        break;

    case 0x6000:
        exec_6xkk(state);
        break;
    case 0x7000:
        exec_7xkk(state);
        break;
    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0x0000:
            exec_8xy0(state);
            break;
        case 0x0001:
            exec_8xy1(state);
            break;
        case 0x0002:
            exec_8xy2(state);
            break;
        case 0x0003:
            exec_8xy3(state);
            break;
        case 0x0004:
            exec_8xy4(state);
            break;
        case 0x0005:
            exec_8xy5(state);
            break;
        case 0x0006:
            exec_8xy6(state);
            break;
        case 0x0007:
            exec_8xy7(state);
            break;
        case 0x000E:
            exec_8xye(state);
            break;
        default:
            return CHIP8_OPCODE_INVALID;
        }
        break;
    case 0x9000:
        switch (opcode & 0x000F)
        {
        case 0:
            exec_9xy0(state);
            break;
        default:
            return CHIP8_OPCODE_INVALID;
        }
        break;
    case 0xA000:
        exec_annn(state);
        break;
    case 0xB000:
        exec_bnnn(state);
        break;
    case 0xC000:
        exec_cxkk(state);
        break;
    case 0xD000:
        exec_dxyn(state);
        break;
    case 0xE000:
        switch (opcode & 0x00FF)
        {
        case 0x9E:
            exec_ex9e(state);
            break;
        case 0xA1:
            exec_exa1(state);
            break;
        default:
            return CHIP8_OPCODE_INVALID;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x07:
            exec_fx07(state);
            break;
        case 0x0A:
            exec_fx0a(state);
            break;
        case 0x15:
            exec_fx15(state);
            break;
        case 0x18:
            exec_fx18(state);
            break;
        case 0x1E:
            exec_fx1e(state);
            break;
        case 0x29:
            exec_fx29(state);
            break;
        case 0x33:
            exec_fx33(state);
            break;
        case 0x55:
            exec_fx55(state);
            break;
        case 0x65:
            exec_fx65(state);
            break;
        default:
            return CHIP8_OPCODE_INVALID;
        }
        break;
    }

    // All instructions count as one cycle
    state->cycle_counts++;

    // Decrement timer at 60Hz, regardless of emulation clock speed.
    int every = state->clock_speed / 60;
    if (state->cycle_counts % every == 0)
    {
        if (state->DT > 0)
            state->DT--;

        if (state->ST > 0)
            state->ST--;
    }

    return CHIP8_OK;
}
