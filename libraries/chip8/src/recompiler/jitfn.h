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
	BH = 7,
	R8L = 8,
	R9L = 9,
	R10L = 10,
	R11L = 11,
	R12L = 12,
	R13L = 13,
	R14L = 14,
	R15L = 15
} Register;

typedef struct {
    uint8_t* buffer;
    uint32_t buffer_size;
    uint32_t buffer_ptr;
    bool executable;
} JitFn;

int jitfn_init(JitFn* fn, uint32_t size);
int jitfn_lock(JitFn* fn);
int jitfn_run(JitFn* fn);

void jitfn_append_ret(JitFn* fn);
void jitfn_append_mov_immtoreg(JitFn* fn, uint8_t src_imm, Register dst_reg);
void jitfn_append_mov_regtomem(JitFn* fn, Register src_reg, uint8_t* dst_addr);
void jitfn_append_mov_memtoreg(JitFn* fn, uint8_t* src_addr, Register dst_reg);
