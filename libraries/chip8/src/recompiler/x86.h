#pragma once
#include <inttypes.h>

typedef enum
{
	AL = 0,
	CL = 1,
	DL = 2,
	BL = 3,
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

void x86_mov_regimm32(X86fn* func, X86reg dst_reg, uint32_t src_imm);
void x86_ret(X86fn* func);
