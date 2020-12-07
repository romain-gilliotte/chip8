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


// /**
//  * Depending on opcode.direction ("d field")
//  *      0 => mob reg2, reg1 
//  *      1 => mov reg1, reg2
//  */
// static void append_regdirect(CodeBlock* func, X86reg reg1, X86reg reg2) {
//     uint8_t modrm = (0b11 << 6) | (reg1 << 3) | reg2;

//     if (reg1 < 8 && reg2 < 8) {
        
//     }

    

//     push_byte(modrm)
// }




/**
 * The ModR/M byte allows to refer to a register or is a part of an instruction used when a memory operand is used.
 * Certain encodings of the ModR/M byte require a second addressing byte (the SIB byte).
 * 
 * Mod is 2 bits, reg and rm are 3 bits.
 *
 * When mod == b11 -> register-direct addressing mode is used
 *          != b11 -> register-indirect addressing mode is used
 *      
 * @example modrm(mod=0b11, reg=AL, rm=0)
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


/**
 * In the ModR/M byte, reg being 3 bits is inconvenient, because we can't access registers R8L->R15L
 * In order to do so, we need to use the "REX" prefix.
 * 
 * The same goes for the MODRM.rm, SIG.index and SIB.base fields, which are extended here.
 * 
 * - W: When 1, a 64-bit operand size is used. Otherwise, when 0, the default operand size is used.
 * - R: This 1-bit value is an extension to the MODRM.reg field.
 * - X: This 1-bit value is an extension to the SIB.index field.
 * - B: This 1-bit value is an extension to the MODRM.rm field or the SIB.base field.
 * 
 * @see https://wiki.osdev.org/X86-64_Instruction_Encoding#REX_prefix
 */
static void push_rexprefix(X86fn* func, bool w, bool r, bool x, bool b) {
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

    if ((int) func->buffer == -1) {
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
    void (*fn)() = (void(*)()) func->buffer;
    fn();

    return 0;
}

/////////
// Instructions
/////////


void x86_mov_regimm32(X86fn* func, X86reg dst_reg, uint32_t src_imm) {
    push_byte(func, 0xB8 | dst_reg);
    push_dword(func, src_imm);
}

void x86_retn(X86fn* func) {
    push_byte(func, 0xC3);
}
