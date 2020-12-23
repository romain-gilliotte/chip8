#include <sys/mman.h>
#include <stdio.h>
#include "x64.h"

/////////
// Helpers
/////////

/** Append a byte, and increment pointer */ 
static void push_byte(X86fn* func, uint8_t byte) {
	func->buffer[func->buffer_ptr++] = byte;
}

/** Append a dword, and increment pointer */
static void push_dword(X86fn* func, uint32_t dword) {
	*((uint32_t*)(func->buffer + func->buffer_ptr)) = dword;
	func->buffer_ptr += 4;
}

/** Append a qword, and increment pointer */
static void push_qword(X86fn* func, uint64_t qword) {
	*((uint64_t*)(func->buffer + func->buffer_ptr)) = qword;
	func->buffer_ptr += 8;
}

/**
 * The ModR/M byte allows to refer to a register or is a part of an instruction used when a memory operand is used.
 * Certain encodings of the ModR/M byte require a second addressing byte (the SIB byte).
 * 
 * Mod is 2 bits, reg and rm are 3 bits.
 *
 * When mod == b11 -> register-direct addressing mode is used
 *          != b11 -> register-indirect addressing mode is used
 *      
 * @example modrm(mod=0b11, reg=EAX, rm=0)
 * 
 * @see https://datacadamia.com/lang/assembly/intel/modrm
 * @see https://wiki.osdev.org/X86-64_Instruction_Encoding#ModR.2FM
 */
static void push_modrm(X86fn* func, uint8_t mod, uint8_t rm, X86reg reg)
{
    uint8_t byte = (mod << 6) | (reg << 3) | rm;
	push_byte(func, byte);
}

// /**
//  * @see https://datacadamia.com/lang/assembly/intel/modrm
//  * @see https://wiki.osdev.org/X86-64_Instruction_Encoding#SIB
//  */
// static void push_sibsb(X86fn* func, uint8_t sib, uint8_t rm, uint8_t index)
// {
//     uint8_t byte = (sib << 6) | (rm << 4) | index;
// 	   push_byte(func, byte);
// }


static void push_rex(X86fn* func, bool w, bool r, bool x, bool b) {
    uint8_t byte = 64 | (w << 3) | (r << 2) | (x << 1) | b;
    push_byte(func, byte);
}


static void push_opmemreg(X86fn* func, uint8_t opcode, X86reg reg, X86reg ptr, int32_t displacement) {
    // rex prefix only needed if we use R8...15 registers
    bool rex_b = ptr >> 3;
    bool rex_r = reg >> 3;
    if (rex_b || rex_r) {
        push_rex(func, 0, rex_r, 0, rex_b);
    }

    // instruction
    push_byte(func, opcode);
    
    // modrm + displacement
    reg = (X86reg) (reg & 0x7);
    ptr = (X86reg) (ptr & 0x7);
    if (displacement == 0) {
        push_modrm(func, 0, ptr, reg);
    }
    else if (displacement < 256) { // compiler seems to consider the limit is 127
        push_modrm(func, 1, ptr, reg);
        push_byte(func, displacement); // disp8
    }
    else {
        push_modrm(func, 2, ptr, reg);
        push_dword(func, displacement); // disp32
    }
}


static void push_opregreg64(X86fn* func, uint8_t opcode, X86reg reg, X86reg ptr) {
    // rex prefix only needed if we use R8...15 registers
    bool rex_b = ptr >> 3;
    bool rex_r = reg >> 3;
    push_rex(func, 1, rex_r, 0, rex_b);

    // instruction
    push_byte(func, opcode);
    
    // modrm + displacement
    reg = (X86reg) (reg & 0x7);
    ptr = (X86reg) (ptr & 0x7);
    push_modrm(func, 3, ptr, reg);
}


static void push_opregreg(X86fn* func, uint8_t opcode, X86reg reg, X86reg ptr) {
    // rex prefix only needed if we use R8...15 registers
    bool rex_b = ptr >> 3;
    bool rex_r = reg >> 3;
    if (rex_b || rex_r) {
        push_rex(func, 0, rex_r, 0, rex_b);
    }

    // instruction
    push_byte(func, opcode);
    
    // modrm + displacement
    reg = (X86reg) (reg & 0x7);
    ptr = (X86reg) (ptr & 0x7);
    push_modrm(func, 3, ptr, reg);
}

/////////
// Init & run
/////////

int x64_init(X86fn* func, uint32_t size) {
    func->buffer = (uint8_t*) mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    func->buffer_ptr = 0;
    func->buffer_size = size;
    func->executable = false;

    if (func->buffer == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    return 0;
}

int x64_lock(X86fn* func) {
    if (mprotect(func->buffer, func->buffer_size, PROT_READ | PROT_EXEC) == -1) {
        perror("mprotect");
        return -1;
    }

    func->executable = true;
    return 0;
}

int x64_run(X86fn* func) {
    if (!func->executable) {
        return -1;
    }

    // What about calling conventions?
    int (*fn)() = (int (*)()) (void*) func->buffer;
    return fn();
}

/////////
// Instructions
/////////

// Immediate

void x64_mov_regimm32(X86fn* func, X86reg reg, uint32_t imm) {
    bool rex_r = reg >> 3;
    if (rex_r)
        push_rex(func, 0, rex_r, 0, 0);

    reg = (X86reg) (reg & 0x7);
    push_byte(func, 0xB8 | reg);
    push_dword(func, imm);
}

void x64_mov_regimm64(X86fn* func, X86reg reg, uint64_t imm) {
    bool rex_r = reg >> 3;
    reg = (X86reg) (reg & 0x7);

    push_rex(func, 1, rex_r, 0, 0);
    push_byte(func, 0xB8 | reg); // mov r16/32/64, imm16/32/64
    push_qword(func, imm);
}

// Move

void x64_movzx_regmem8(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement) {
    // the prefix should be between REX and primary opcode
    bool rex_b = ptr >> 3;
    bool rex_r = reg >> 3;
    push_rex(func, 1, rex_r, 0, rex_b);
    push_byte(func, 0x0f); // this will break if used with r8 and more.
    push_opmemreg(func, 0xB6, reg, ptr, displacement);
}

void x64_movzx_regmem16(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement) {
    // this will break if used with r8 and more.
    // the prefix should be between REX and primary opcode
    bool rex_b = ptr >> 3;
    bool rex_r = reg >> 3;
    push_rex(func, 1, rex_r, 0, rex_b);
    push_byte(func, 0x0f);
    push_opmemreg(func, 0xB7, reg, ptr, displacement);
}


void x64_mov_regmem8(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0x8A, reg, ptr, displacement);
}

void x64_mov_regmem16(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement) {
    push_byte(func, 0x66); //  Operand-Size prefix
    push_opmemreg(func, 0x8B, reg, ptr, displacement);
}

void x64_mov_regmem32(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0x8B, reg, ptr, displacement);
}

void x64_mov_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_opmemreg(func, 0x88, reg, ptr, displacement);
}

void x64_mov_memreg16(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_byte(func, 0x66); //  Operand-Size prefix
    push_opmemreg(func, 0x89, reg, ptr, displacement);
}

void x64_mov_memreg32(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_opmemreg(func, 0x89, reg, ptr, displacement);
}

void x64_retn(X86fn* func) {
    push_byte(func, 0xC3);
}

// add
void x64_add_aximm8(X86fn* func, uint8_t imm) {
    push_byte(func, 0x66);
    push_byte(func, 0x83);
    push_byte(func, 0xc0);
    push_byte(func, imm);
}

void x64_add_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_opmemreg(func, 0x00, reg, ptr, displacement);
}

void x64_add_memreg16(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_byte(func, 0x66); //  Operand-Size prefix
    push_opmemreg(func, 0x01, reg, ptr, displacement);
}

void x64_add_memreg32(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_opmemreg(func, 0x01, reg, ptr, displacement);
}

// inc/dec

void x64_inc_mem8(X86fn* func, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0xfe, 0, ptr, displacement);
}

void x64_dec_mem8(X86fn* func, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0xfe, 1, ptr, displacement);
}

void x64_inc_mem32(X86fn* func, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0xff, 0, ptr, displacement);    
}

void x64_dec_mem32(X86fn* func, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0xff, 1, ptr, displacement);    
}

// others, 8 bits

void x64_cmp_regmem8(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0x3A, reg, ptr, displacement);
}

void x64_or_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_opmemreg(func, 0x08, reg, ptr, displacement);
}

void x64_and_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_opmemreg(func, 0x20, reg, ptr, displacement);
}

void x64_xor_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_opmemreg(func, 0x30, reg, ptr, displacement);
}

// jumps
void x64_jz8(X86fn* func, int8_t distance) {
    push_byte(func, 0x74);
    push_byte(func, (uint8_t) distance);
}

void x64_jnz8(X86fn* func, int8_t distance) {
    push_byte(func, 0x75);
    push_byte(func, (uint8_t) distance);
}


void x64_setc(X86fn* func, X86reg ptr, int32_t displacement) {
    push_byte(func, 0x0F);
    push_byte(func, 0x92);
    
    if (displacement < 256) { // compiler seems to consider the limit is 127
        push_modrm(func, 1, ptr, 0);
        push_byte(func, displacement); // disp8
    }
    else {
        push_modrm(func, 2, ptr, 0);
        push_dword(func, displacement); // disp32
    }
}

void x64_setnc(X86fn* func, X86reg ptr, int32_t displacement) {
    push_byte(func, 0x0F);
    push_byte(func, 0x93);
    
    if (displacement < 256) { // compiler seems to consider the limit is 127
        push_modrm(func, 1, ptr, 0);
        push_byte(func, displacement); // disp8
    }
    else {
        push_modrm(func, 2, ptr, 0);
        push_dword(func, displacement); // disp32
    }
}

void x64_sub_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg) {
    push_opmemreg(func, 0x28, reg, ptr, displacement);
}

void x64_shr_memreg8(X86fn* func, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0xd0, 5, ptr, displacement);
}

void x64_shl_memreg8(X86fn* func, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0xd0, 4, ptr, displacement);
}

void x64_sub_regreg8(X86fn* func, X86reg ptr, X86reg reg) {
    push_opregreg(func, 0x28, reg, ptr);
}

void x64_sub_regmem8(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement) {
    push_opmemreg(func, 0x2A, reg, ptr, displacement);
}

void x64_add_regreg64(X86fn* func, X86reg reg, X86reg ptr) {
    push_opregreg64(func, 0x03, reg, ptr);
}
