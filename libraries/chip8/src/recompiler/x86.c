#include <sys/mman.h>
#include <stdio.h>
#include "x86.h"

/////////
// Helpers
/////////

/** Append a byte, and increment pointer */ 
static void push_byte(X86fn* func, uint8_t byte) {
	func->buffer[func->buffer_ptr++] = byte;
}

/** Append a word, and increment pointer */
static void push_word(X86fn* func, uint16_t word) {
	*((uint16_t*)(func->buffer + func->buffer_ptr)) = word;
	func->buffer_ptr += 2;
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
    uint8_t byte = (mod << 6) | (reg << 4) | rm;

	push_byte(func, byte);
}

/**
 * @see https://datacadamia.com/lang/assembly/intel/modrm
 * @see https://wiki.osdev.org/X86-64_Instruction_Encoding#SIB
 */
static void push_sibsb(X86fn* func, uint8_t sib, uint8_t rm, uint8_t index)
{
    uint8_t byte = (sib << 6) | (rm << 4) | index;
	push_byte(func, byte);
}


static void push_rex(X86fn* func, bool w, bool r, bool x, bool b) {
    uint8_t byte = 0b01000000 | (w << 3) | (r << 2) | (x << 1) | b;
    push_byte(func, byte);
}

/////////
// Init & run
/////////

int x86_init(X86fn* func, uint32_t size) {
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

int x86_lock(X86fn* func) {
    if (mprotect(func->buffer, func->buffer_size, PROT_READ | PROT_EXEC) == -1) {
        perror("mprotect");
        return -1;
    }

    func->executable = true;
}

int x86_run(X86fn* func) {
    if (!func->executable) {
        return -1;
    }

    // What about calling conventions?
    int (*fn)() = (int (*)()) func->buffer;
    return fn();
}

/////////
// Instructions
/////////

void x86_mov_regimm32(X86fn* func, X86reg reg, uint32_t imm) {
    bool rex_r = reg >> 3;
    reg = reg & 0x1F;

    if (rex_r)
        push_rex(func, 0, rex_r, 0, 0);    

    push_byte(func, 0xB8 | reg);
    push_dword(func, imm);
}

void x86_mov_regimm64(X86fn* func, X86reg reg, uint64_t imm) {
    bool rex_r = reg >> 3;
    reg = reg & 0x1F;

    push_rex(func, 1, rex_r, 0, 0);
    push_byte(func, 0xB8 | reg); // mov r16/32/64, imm16/32/64
    push_qword(func, imm);
}

void x86_mov_regmem8(X86fn* func, X86reg reg, X86reg ptr, int16_t displacement) {
    // rex prefix only needed if we use R8...15 registers
    bool rex_b = ptr >> 3;
    bool rex_r = ptr >> 3;
    reg = reg & 0x1F;
    ptr = ptr & 0x1F;

    if (rex_b || rex_r) {
        push_rex(func, 0, rex_r, 0, rex_b);
    }

    // instruction
    push_byte(func, 0x8A);
    
    // modrm + displacement
    if (displacement < 256) { // compiler seems to consider the limit is 127
        push_modrm(func, 0b01, ptr, reg);
        push_byte(func, displacement); // disp8
    }
    else {
        push_modrm(func, 0b10, ptr, reg);
        push_dword(func, displacement); // disp32
    }
}

void x86_mov_memreg8(X86fn* func, X86reg ptr, int16_t displacement, X86reg reg) {
    // rex prefix only needed if we use R8...15 registers
    bool rex_b = ptr >> 3;
    bool rex_r = ptr >> 3;
    if (rex_b || rex_r) {
        push_rex(func, 0, rex_r, 0, rex_b);
    }

    // instruction
    push_byte(func, 0x88);
    
    // modrm + displacement
    if (displacement < 256) { // compiler seems to consider the limit is 127
        push_modrm(func, 0b01, ptr, reg);
        push_byte(func, displacement); // disp8
    }
    else {
        push_modrm(func, 0b10, ptr, reg);
        push_dword(func, displacement); // disp32
    }
}

void x86_add_regimm8(X86fn* func, X86reg reg, uint8_t imm) {
    if (reg == EAX) {
        push_byte(func, 0x04);
        push_byte(func, imm);
    }
    else {
        push_byte(func, 0x02);
        push_modrm(func, 0b00, 0b101, reg);
        push_byte(func, imm);
    }
}

void x86_retn(X86fn* func) {
    push_byte(func, 0xC3);
}
