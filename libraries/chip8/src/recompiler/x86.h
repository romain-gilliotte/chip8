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

// Move data from and to registers.
void x86_mov_regimm32(X86fn* func, X86reg reg, uint32_t imm);
void x86_mov_regimm64(X86fn* func, X86reg reg, uint64_t imm);

void x86_mov_regmem8(X86fn* func, X86reg reg, X86reg ptr, int16_t displacement);
void x86_mov_memreg8(X86fn* func, X86reg ptr, int16_t displacement, X86reg reg);

void x86_add_regimm8(X86fn* func, X86reg reg, uint8_t imm);
