#include <inttypes.h>
#include "cache.h"

int create_cache(Chip8* state, CodeCache* cache) {
    cache->start = state->PC;
    cache->end = state->PC;

    jitfn_init(&cache->code, 4096);

    while (!cache->code.executable) {
        uint16_t opcode = *(uint16_t*) (state->memory + cache->end);

        encode(&cache->code, opcode);
        cache->end += 2;
    }
}


static int encode(JitFn* jitfn, uint16_t opcode) {
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}


/**
 * 00E0 - CLS
 * Clear the display.
 */
static int encode_00e0(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 00EE - RET
 * Return from a subroutine.
 * 
 * The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
 */
static int encode_00ee(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 1nnn - JP addr
 * Jump to location nnn.
 * 
 * The interpreter sets the program counter to nnn.
 */
static int encode_1nnn(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 2nnn - CALL addr
 * Call subroutine at nnn.
 * 
 * The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
 */
static int encode_2nnn(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 3xkk - SE Vx, byte
 * Skip next instruction if Vx = kk.
 * 
 * The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
 */
static int encode_3xkk(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 4xkk - SNE Vx, byte
 * Skip next instruction if Vx != kk.
 * 
 * The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
 */
static int encode_4xkk(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 5xy0 - SE Vx, Vy
 * Skip next instruction if Vx = Vy.
 * 
 * The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
 */
static int encode_5xy0(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 6xkk - LD Vx, byte
 * Set Vx = kk.
 * 
 * The interpreter puts the value kk into register Vx.
 */
static int encode_6xkk(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 7xkk - ADD Vx, byte
 * Set Vx = Vx + kk.
 * 
 * Adds the value kk to the value of register Vx, then stores the result in Vx.
 */
static int encode_7xkk(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 8xy0 - LD Vx, Vy
 * Set Vx = Vy.
 * 
 * Stores the value of register Vy in register Vx.
 */
static int encode_8xy0(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 8xy1 - OR Vx, Vy
 * Set Vx = Vx OR Vy.
 * 
 * Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. 
 * A bitwise OR compares the corrseponding bits from two values, and if either bit is 1,
 * then the same bit in the result is also 1. Otherwise, it is 0.
 */
static int encode_8xy1(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 8xy2 - AND Vx, Vy
 * Set Vx = Vx AND Vy.
 * 
 * Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
 * A bitwise AND compares the corrseponding bits from two values, and if both bits are 1,
 * then the same bit in the result is also 1. Otherwise, it is 0.
 */
static int encode_8xy2(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 8xy3 - XOR Vx, Vy
 * Set Vx = Vx XOR Vy.
 * 
 * Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
 * An exclusive OR compares the corrseponding bits from two values, and if the bits are not 
 * both the same, then the corresponding bit in the result is set to 1. Otherwise, it is 0.
 */
static int encode_8xy3(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 8xy4 - ADD Vx, Vy
 * Set Vx = Vx + Vy, set VF = carry.
 * 
 * The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,)
 * VF is set to 1, otherwise 0. 
 * Only the lowest 8 bits of the result are kept, and stored in Vx.
 */
static int encode_8xy4(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 8xy5 - SUB Vx, Vy
 * Set Vx = Vx - Vy, set VF = NOT borrow.
 * 
 * If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
 */
static int encode_8xy5(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 8xy6 - SHR Vx, Vy
 * Set Vx = Vx SHR 1.
 * 
 * If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
 */
static int encode_8xy6(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 8xy7 - SUBN Vx, Vy
 * Set Vx = Vy - Vx, set VF = NOT borrow.
 * 
 * If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
 */
static int encode_8xy7(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 8xyE - SHL Vx, Vy
 * Set Vx = Vx SHL 1.
 * 
 * If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
 */
static int encode_8xye(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * 9xy0 - SNE Vx, Vy
 * Skip next instruction if Vx != Vy.
 * 
 * The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
 */
static int encode_9xy0(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Annn - LD I, addr
 * Set I = nnn.
 * 
 * The value of register I is set to nnn.
 */
static int encode_annn(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Bnnn - JP V0, addr
 * Jump to location nnn + V0.
 * 
 * The program counter is set to nnn plus the value of V0.
 */
static int encode_bnnn(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Cxkk - RND Vx, byte
 * Set Vx = random byte AND kk.
 * 
 * The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.
 * The results are stored in Vx. See instruction 8xy2 for more information on AND.
 */
static int encode_cxkk(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
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
static int encode_dxyn(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Ex9E - SKP Vx
 * Skip next instruction if key with the value of Vx is pressed.
 * 
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
 */
static int encode_ex9e(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * ExA1 - SKNP Vx
 * Skip next instruction if key with the value of Vx is not pressed.
 * 
 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
 */
static int encode_exa1(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Fx07 - LD Vx, DT
 * Set Vx = delay timer value.
 * 
 * The value of DT is placed into Vx.
 */
static int encode_fx07(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Fx0A - LD Vx, K
 * Wait for a key press, store the value of the key in Vx.
 * 
 * All execution stops until a key is pressed, then the value of that key is stored in Vx.
 */
static int encode_fx0a(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Fx15 - LD DT, Vx
 * Set delay timer = Vx.
 * 
 * DT is set equal to the value of Vx.
 */
static int encode_fx15(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Fx18 - LD ST, Vx
 * Set sound timer = Vx.
 * 
 * ST is set equal to the value of Vx.
 */
static int encode_fx18(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Fx1E - ADD I, Vx
 * Set I = I + Vx.
 * 
 * The values of I and Vx are added, and the results are stored in I.
 */
static int encode_fx1e(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Fx29 - LD F, Vx
 * Set I = location of sprite for digit Vx.
 * 
 * The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
 * See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
 */
static int encode_fx29(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Fx33 - LD B, Vx
 * Store BCD representation of Vx in memory locations I, I+1, and I+2.
 * 
 * The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I,
 * the tens digit at location I+1, and the ones digit at location I+2.
 */
static int encode_fx33(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Fx55 - LD [I], Vx
 * Store registers V0 through Vx in memory starting at location I.
 * 
 * The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
 */
static int encode_fx55(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}

/**
 * Fx65 - LD Vx, [I]
 * Read registers V0 through Vx from memory starting at location I.
 * 
 * The interpreter reads values from memory starting at location I into registers V0 through Vx.
 */
static int encode_fx65(JitFn* jitfn, uint16_t opcode)
{
    jitfn_append_mov_immtoreg(jitfn, USE_INTERPRETER, AL);
    jitfn_append_ret(jitfn);
    jitfn_lock(jitfn);
}
