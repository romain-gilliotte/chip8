#pragma once
#include <inttypes.h>
#include <stdbool.h>

typedef enum
{
	EAX = 0,
	ECX = 1,
	EDX = 2,
	EBX = 3,
	AH = 4,
	CH = 5,
	DH = 6,
	BH = 7
} X86reg;

typedef struct {
    uint8_t* buffer;
    uint32_t buffer_size;
    uint32_t buffer_ptr;
    bool executable;
} X86fn;

int x86_init(X86fn* fn, uint32_t size);
int x86_lock(X86fn* fn);
int x86_run(X86fn* fn);

void x86_retn(X86fn* func);

//////////
// Move
//////////

// reg <- immediate
void x86_mov_regimm32(X86fn* func, X86reg reg, uint32_t imm);
void x86_mov_regimm64(X86fn* func, X86reg reg, uint64_t imm);

// reg <- memory
void x86_mov_regmem8(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement);
void x86_mov_regmem16(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement);
void x86_mov_regmem32(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement);
void x86_movzx_regmem8(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement);

// memory <- reg
void x86_mov_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg);
void x86_mov_memreg16(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg);

//////////
// Add
//////////

void x86_add_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg);
void x86_add_memreg16(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg);
void x86_add_memreg32(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg);

//////////
// Inc/Dec
//////////

void x86_inc_mem32(X86fn* func, X86reg ptr, int32_t displacement);
void x86_dec_mem32(X86fn* func, X86reg ptr, int32_t displacement);

//////////
// 8 bits ops
//////////

void x86_cmp_regmem8(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement);

void x86_or_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg);
void x86_and_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg);
void x86_xor_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg);


//////////
// jumps
//////////

void x86_jz8(X86fn* func, int8_t distance);
void x86_jnz8(X86fn* func, int8_t distance);


void x86_setc(X86fn* func, X86reg ptr, int32_t displacement);
void x86_setnc(X86fn* func, X86reg ptr, int32_t displacement);

void x86_sub_memreg8(X86fn* func, X86reg ptr, int32_t displacement, X86reg reg);
void x86_shr_memreg8(X86fn* func, X86reg ptr, int32_t displacement);
void x86_shl_memreg8(X86fn* func, X86reg ptr, int32_t displacement);

void x86_sub_regreg8(X86fn* func, X86reg ptr, X86reg reg);
void x86_sub_regmem8(X86fn* func, X86reg reg, X86reg ptr, int32_t displacement);