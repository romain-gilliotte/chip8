#include <sys/mman.h>
#include <stdio.h>
#include "jitfn.h"

/////////
// Helpers
/////////

/** Append a byte, and increment pointer */ 
static void append_byte(JitFn* jitfn, uint8_t byte) {
	jitfn->buffer[jitfn->buffer_ptr++] = byte;
}

/** Append a word, and increment pointer */
static void append_word(JitFn* jitfn, uint16_t word) {
	*((uint16_t*)(jitfn->buffer + jitfn->buffer_ptr)) = word;
	jitfn->buffer_ptr += 2;
}

/** Append a dword, and increment pointer */
static void append_dword(JitFn* jitfn, uint32_t dword) {
	*((uint32_t*)(jitfn->buffer + jitfn->buffer_ptr)) = dword;
	jitfn->buffer_ptr += 4;
}


// /**
//  * Depending on opcode.direction ("d field")
//  *      0 => mob reg2, reg1 
//  *      1 => mov reg1, reg2
//  */
// static void append_regdirect(CodeBlock* jitfn, Register reg1, Register reg2) {
//     uint8_t modrm = (0b11 << 6) | (reg1 << 3) | reg2;

//     if (reg1 < 8 && reg2 < 8) {
        
//     }

    

//     append_byte(modrm)
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
static void append_modrm(JitFn* jitfn, uint8_t mod, uint8_t rm, Register reg)
{
    uint8_t byte = (mod << 6) | (reg << 4) | rm;

	append_byte(jitfn, byte);
}

/**
 * @see https://datacadamia.com/lang/assembly/intel/modrm
 * @see https://wiki.osdev.org/X86-64_Instruction_Encoding#SIB
 */
static void append_sibsb(JitFn* jitfn, uint8_t sib, uint8_t rm, uint8_t index)
{
    uint8_t byte = (sib << 6) | (rm << 4) | index;
	append_byte(jitfn, byte);
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
static void append_rexprefix(JitFn* jitfn, bool w, bool r, bool x, bool b) {
    uint8_t byte = 0b01000000 | (w << 3) | (r << 2) | (x << 1) | b;
	append_byte(jitfn, byte);
}


/////////
// Init & run
/////////

int jitfn_init(JitFn* jitfn, uint32_t size) {
    jitfn->buffer = (uint8_t*) mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    jitfn->buffer_ptr = 0;
    jitfn->buffer_size = size;

    if ((int) jitfn->buffer == -1) {
        perror("mmap");
        return -1;
    }

    return 0;
}

int jitfn_lock(JitFn* jitfn) {
    if (mprotect(jitfn->buffer, jitfn->buffer_size, PROT_READ | PROT_EXEC) == -1) {
        perror("mprotect");
        return -1;
    }

    jitfn->executable = true;
}

int jitfn_run(JitFn* jitfn) {
    if (!jitfn->executable) {
        return -1;
    }

    // What about calling conventions?
    void (*fn)() = (void(*)()) jitfn->buffer;
    fn();

    return 0;
}

/////////
// Instructions
/////////


void jitfn_append_mov_immtoreg(JitFn* jitfn, uint8_t src_imm, Register dst_reg) {
}

void cb_append_mov_regtomem(JitFn* jitfn, Register src_reg, uint32_t dst_addr) {

}

void cb_append_mov_memtoreg(JitFn* jitfn, uint32_t src_addr, Register dst_reg) {

}


void jitfn_append_ret(JitFn* jitfn) {

}
