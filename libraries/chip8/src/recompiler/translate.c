#include <stddef.h>
#include "translate.h"


/** 
 * Compute length of next instruction in x64.
 * This is needed to compute relative jumps when translating skip instructions
 */
static int next_length(CodeCache* cache, Chip8* state) {
    cache->end += 2;

    int buffer_ptr = cache->code.buffer_ptr; // Save pointer
    translate_instruction(cache, state); // Write instruction

    int length = cache->code.buffer_ptr - buffer_ptr; // compute length

    // Restore cache.
    cache->code.buffer_ptr = buffer_ptr;
    cache->end -= 2;
    return length;
}

/**
 * Encode x64 for errors (invalid/unsupported opcode).
 * 
 * This simply make the generated function update the state (PC & elapsed cycles)
 * and return the error to the caller.
 */
static bool encode_error(CodeCache* cache, Chip8Error code) {
    if (cache->start < cache->end) {
        // Update program counter.
        x64_mov_regimm32(&cache->code, EAX, cache->end);
        x64_mov_memreg16(&cache->code, ECX, offsetof(Chip8, PC), EAX);

        // Update elapsed cycles
        x64_mov_regimm32(&cache->code, EAX, (cache->end - cache->start) / 2);
        x64_add_memreg32(&cache->code, ECX, offsetof(Chip8, cycles_since_started), EAX);
    }

    // return error code.
    x64_mov_regimm32(&cache->code, EAX, code);
    x64_retn(&cache->code);

    return true;
}

static bool encode_invalid(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) opcode, (void) state;
    return encode_error(cache, CHIP8_OPCODE_INVALID);
}

static bool encode_not_supported(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) opcode, (void) state;
    return encode_error(cache, CHIP8_OPCODE_NOT_SUPPORTED);
}

static bool encode_ret(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) opcode, (void) state;

    // state->SP--
    x64_dec_mem8(&cache->code, ECX, offsetof(Chip8, SP));

    // Compute pointer &state + SP*2
    x64_movzx_regmem8(&cache->code, EDX, ECX, offsetof(Chip8, SP)); // sp dans rdx
    x64_add_regreg64(&cache->code, EDX, EDX); // multiplie par 2
    x64_add_regreg64(&cache->code, EDX, ECX); // &state dans rdx

    // Update PC and cycles
    x64_mov_regmem16(&cache->code, EAX, EDX, offsetof(Chip8, stack)); // ax = [state + stack + 2*sp]
    x64_add_aximm8(&cache->code, 2); // ax += 2
    x64_mov_memreg16(&cache->code, ECX, offsetof(Chip8, PC), EAX);

    x64_mov_regimm32(&cache->code, EAX, 1 + (cache->end - cache->start) / 2);
    x64_add_memreg32(&cache->code, ECX, offsetof(Chip8, cycles_since_started), EAX);

    // return CHIP8_OK
    x64_mov_regimm32(&cache->code, EAX, CHIP8_OK);
    x64_retn(&cache->code);
    return true;
}

static bool encode_jmp_nnn(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;

    // Update PC and cycles
    x64_mov_regimm32(&cache->code, EAX, opcode->nnn);
    x64_mov_memreg16(&cache->code, ECX, offsetof(Chip8, PC), EAX);
    x64_mov_regimm32(&cache->code, EAX, 1 + (cache->end - cache->start) / 2);
    x64_add_memreg32(&cache->code, ECX, offsetof(Chip8, cycles_since_started), EAX);

    // return CHIP8_OK
    x64_mov_regimm32(&cache->code, EAX, CHIP8_OK);
    x64_retn(&cache->code);
    return true;
}

static bool encode_call_nnn(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;

    // Compute pointer &state + SP*2
    x64_movzx_regmem8(&cache->code, EDX, ECX, offsetof(Chip8, SP)); // rdx = sp
    x64_add_regreg64(&cache->code, EDX, EDX); // rdx *= 2
    x64_add_regreg64(&cache->code, EDX, ECX); // rdx += &state

    // Save PC
    x64_mov_regimm32(&cache->code, EAX, cache->end);
    x64_mov_memreg16(&cache->code, EDX, offsetof(Chip8, stack), EAX); // &state + offsetof(Chip8, stack) + 2*sp = pc

    // state->sp++
    x64_inc_mem8(&cache->code, ECX, offsetof(Chip8, SP));

    // Update PC and cycles
    x64_mov_regimm32(&cache->code, EAX, opcode->nnn);
    x64_mov_memreg16(&cache->code, ECX, offsetof(Chip8, PC), EAX);
    x64_mov_regimm32(&cache->code, EAX, 1 + (cache->end - cache->start) / 2);
    x64_add_memreg32(&cache->code, ECX, offsetof(Chip8, cycles_since_started), EAX);

    // return CHIP8_OK
    x64_mov_regimm32(&cache->code, EAX, CHIP8_OK);
    x64_retn(&cache->code);
    return true;
}

static bool encode_se_vx_kk(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    x64_dec_mem32(&cache->code, ECX, offsetof(Chip8, cycles_since_started)); // cycles--
    x64_mov_regimm32(&cache->code, EAX, opcode->kk); // load kk in register
    x64_cmp_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->x); // cmp Vx, kk
    x64_jz8(&cache->code, 3 + next_length(cache, state)); // jz after next instruction (inc is 3bytes)
    x64_inc_mem32(&cache->code, ECX, offsetof(Chip8, cycles_since_started)); // cycles++
    return false;
}

static bool encode_sne_vx_kk(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    x64_dec_mem32(&cache->code, ECX, offsetof(Chip8, cycles_since_started)); // cycles--
    x64_mov_regimm32(&cache->code, EAX, opcode->kk); // load kk in register
    x64_cmp_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->x); // cmp Vx, kk
    x64_jnz8(&cache->code, 3 + next_length(cache, state)); // jz after next instruction (inc is 3bytes)
    x64_inc_mem32(&cache->code, ECX, offsetof(Chip8, cycles_since_started)); // cycles++
    return false;
}

static bool encode_se_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    x64_dec_mem32(&cache->code, ECX, offsetof(Chip8, cycles_since_started)); // cycles--
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->y); // mov al, [state->registers + y]
    x64_cmp_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->x); // cmp Vx, kk
    x64_jz8(&cache->code, 3 + next_length(cache, state)); // jz after next instruction (inc is 3bytes)
    x64_inc_mem32(&cache->code, ECX, offsetof(Chip8, cycles_since_started)); // cycles++
    return false;
}

static bool encode_ld_vx_kk(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regimm32(&cache->code, EAX, opcode->kk); // mov eax, kk
    x64_mov_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX); // mov [state->registers + x], al
    return false;
}

static bool encode_add_vx_kk(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regimm32(&cache->code, EAX, opcode->kk); // mov eax, kk
    x64_add_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX); // add [state->registers + x], al
    return false;
}

static bool encode_ld_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->y);
    x64_mov_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX);
    return false;
}

static bool encode_or_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->y);
    x64_or_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX);
    return false;
}

static bool encode_and_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->y);
    x64_and_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX);
    return false;
}

static bool encode_xor_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->y);
    x64_xor_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX);
    return false;
}

static bool encode_add_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->y);
    x64_add_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX);
    x64_setc(&cache->code, ECX, offsetof(Chip8, registers) + 15);
    return false;
}

static bool encode_sub_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->y);
    x64_sub_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX);
    x64_setnc(&cache->code, ECX, offsetof(Chip8, registers) + 15); // x > y
    return false;
}

static bool encode_shr_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_shr_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x);
    x64_setc(&cache->code, ECX, offsetof(Chip8, registers) + 15);
    return false;
}

static bool encode_subn_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->y);
    x64_sub_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->x);
    x64_setnc(&cache->code, ECX, offsetof(Chip8, registers) + 15);
    x64_mov_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX);
    return false;
}

static bool encode_shl_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_shl_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x);
    x64_setc(&cache->code, ECX, offsetof(Chip8, registers) + 15);
    return false;
}

static bool encode_sne_vx_vy(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    x64_dec_mem32(&cache->code, ECX, offsetof(Chip8, cycles_since_started)); // cycles--
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->y); // mov al, [state->registers + y]
    x64_cmp_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->x); // cmp Vx, kk
    x64_jnz8(&cache->code, 3 + next_length(cache, state)); // jz after next instruction (inc is 3bytes)
    x64_inc_mem32(&cache->code, ECX, offsetof(Chip8, cycles_since_started)); // cycles++
    return false;
}

static bool encode_ld_i_nnn(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regimm32(&cache->code, EAX, opcode->nnn);
    x64_mov_memreg16(&cache->code, ECX, offsetof(Chip8, I), EAX);
    return false;
}

static bool encode_jp_v0_nnn(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;

    // Update PC and cycles
    x64_mov_regimm32(&cache->code, EAX, opcode->nnn);
    x64_mov_memreg16(&cache->code, ECX, offsetof(Chip8, PC), EAX);
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers)); // not super efficient
    x64_add_memreg16(&cache->code, ECX, offsetof(Chip8, PC), EAX);

    // Update cycles
    x64_mov_regimm32(&cache->code, EAX, 1 + (cache->end - cache->start) / 2);
    x64_add_memreg32(&cache->code, ECX, offsetof(Chip8, cycles_since_started), EAX);

    // return CHIP8_OK
    x64_mov_regimm32(&cache->code, EAX, CHIP8_OK);
    x64_retn(&cache->code);
    return true;
}

static bool encode_ld_vx_dt(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, DT));
    x64_mov_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + opcode->x, EAX);
    return false;
}

static bool encode_ld_vx_k(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) opcode, (void) state;
    return encode_error(cache, CHIP8_OPCODE_NOT_SUPPORTED);
}

static bool encode_ld_dt_vx(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->x);
    x64_mov_memreg8(&cache->code, ECX, offsetof(Chip8, DT), EAX);
    return false;
}

static bool encode_ld_st_vx(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->x);
    x64_mov_memreg8(&cache->code, ECX, offsetof(Chip8, ST), EAX);
    return false;
}

static bool encode_add_i_vx(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;
    x64_movzx_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + opcode->x);
    x64_add_memreg16(&cache->code, ECX, offsetof(Chip8, I), EAX);
    return false;
}

static bool encode_ld_i_vx(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;

    x64_movzx_regmem16(&cache->code, EDX, ECX, offsetof(Chip8, I));
    x64_add_regreg64(&cache->code, EDX, ECX);

    for (uint8_t reg_id = 0; reg_id <= opcode->x; ++reg_id) {
        x64_mov_regmem8(&cache->code, EAX, ECX, offsetof(Chip8, registers) + reg_id);
        x64_mov_memreg8(&cache->code, EDX, offsetof(Chip8, memory) + reg_id, EAX);
    }

    x64_mov_regimm32(&cache->code, EAX, opcode->x + 1);
    x64_add_memreg16(&cache->code, ECX, offsetof(Chip8, I), EAX);
    return false;
}

static bool encode_ld_vx_i(CodeCache* cache, Chip8* state, Chip8Opcode* opcode) {
    (void) state;

    x64_movzx_regmem16(&cache->code, EDX, ECX, offsetof(Chip8, I));
    x64_add_regreg64(&cache->code, EDX, ECX);

    for (uint8_t reg_id = 0; reg_id <= opcode->x; ++reg_id) {
        x64_mov_regmem8(&cache->code, EAX, EDX, offsetof(Chip8, memory) + reg_id);
        x64_mov_memreg8(&cache->code, ECX, offsetof(Chip8, registers) + reg_id, EAX);
    }

    x64_mov_regimm32(&cache->code, EAX, opcode->x + 1);
    x64_add_memreg16(&cache->code, ECX, offsetof(Chip8, I), EAX);
    return false;
}

static bool (*encode_instruction[])(CodeCache*, Chip8*, Chip8Opcode*) = {
    // Original
    encode_invalid,       // OPCODE_INVALID
    encode_not_supported, // OPCODE_CLS,
    encode_ret,           // OPCODE_RET,
    encode_jmp_nnn,       // OPCODE_JMP_NNN,
    encode_call_nnn,      // OPCODE_CALL_NNN,
    encode_se_vx_kk,      // OPCODE_SE_VX_KK,
    encode_sne_vx_kk,     // OPCODE_SNE_VX_KK,
    encode_se_vx_vy,      // OPCODE_SE_VX_VY,
    encode_ld_vx_kk,      // OPCODE_LD_VX_KK,
    encode_add_vx_kk,     // OPCODE_ADD_VX_KK,
    encode_ld_vx_vy,      // OPCODE_LD_VX_VY,
    encode_or_vx_vy,      // OPCODE_OR_VX_VY,
    encode_and_vx_vy,     // OPCODE_AND_VX_VY,
    encode_xor_vx_vy,     // OPCODE_XOR_VX_VY,
    encode_add_vx_vy,     // OPCODE_ADD_VX_VY,
    encode_sub_vx_vy,     // OPCODE_SUB_VX_VY,
    encode_shr_vx_vy,     // OPCODE_SHR_VX_VY,
    encode_subn_vx_vy,    // OPCODE_SUBN_VX_VY,
    encode_shl_vx_vy,     // OPCODE_SHL_VX_VY,
    encode_sne_vx_vy,     // OPCODE_SNE_VX_VY,
    encode_ld_i_nnn,      // OPCODE_LD_I_NNN,
    encode_jp_v0_nnn,     // OPCODE_JP_V0_NNN,
    encode_not_supported, // OPCODE_RND_VX_KK,
    encode_not_supported, // OPCODE_DRW_VX_VY_N,
    encode_not_supported, // OPCODE_SKP_VX,
    encode_not_supported, // OPCODE_SKNP_VX,
    encode_ld_vx_dt,      // OPCODE_LD_VX_DT,
    encode_ld_vx_k,       // OPCODE_LD_VX_K,
    encode_ld_dt_vx,      // OPCODE_LD_DT_VX,
    encode_ld_st_vx,      // OPCODE_LD_ST_VX,
    encode_add_i_vx,      // OPCODE_ADD_I_VX,
    encode_not_supported, // OPCODE_LD_F_VX,
    encode_not_supported, // OPCODE_LD_B_VX,
    encode_not_supported, // OPCODE_LD_I_VX,
    encode_not_supported, // OPCODE_LD_VX_I,

    // S-Chip
    encode_not_supported, // OPCODE_SCRL_DOWN_N,
    encode_not_supported, // OPCODE_SCRL_LEFT,
    encode_not_supported, // OPCODE_SCRL_RIGHT,
    encode_not_supported, // OPCODE_EXIT,
    encode_not_supported, // OPCODE_HIDEF_OFF,
    encode_not_supported, // OPCODE_HIDEF_ON,
    encode_not_supported, // OPCODE_LD_I_,
    encode_not_supported, // OPCODE_LD_RPL_VX,
    encode_not_supported, // OPCODE_LD_VX_RPL,

    // XO-Chip
    encode_not_supported, // OPCODE_LD_I_VX_VY,
    encode_not_supported, // OPCODE_LD_VX_VY_I,
    encode_not_supported, // OPCODE_LD_I_NNNN,
    encode_not_supported, // OPCODE_DRW_PLN_N,
    encode_not_supported, // OPCODE_LD_AUDIO_I,
    encode_not_supported, // OPCODE_SCRL_UP_N,
};


void translate_block(CodeCache* cache, Chip8* state) {
    cache->start = cache->end = state->PC;

    x64_init(&cache->code, 4096);
    x64_mov_regimm64(&cache->code, ECX, (uint64_t) state); // Load pointer to chip8 state

    while (!translate_instruction(cache, state)) {
        cache->end += 2;
    }

    x64_lock(&cache->code);

    // debug print
    for (uint32_t i = 0; i < cache->code.buffer_ptr; ++i)
        printf("%02hhX", cache->code.buffer[i]);
    printf("   %d\n", (cache->end - cache->start) / 2);
}


bool translate_instruction(CodeCache* cache, Chip8* state) {
    // Decode current
    Chip8Opcode opcode;
    chip8_decode(state, &opcode, cache->end);
    
    bool done = encode_instruction[opcode.id](cache, state, &opcode);

    // Instruction just after a skip cannot be the end of a block.
    if (done && cache->end >= 2 && cache->start < cache->end) {
        chip8_decode(state, &opcode, cache->end - 2);

        done = opcode.id != OPCODE_SE_VX_KK
            && opcode.id != OPCODE_SNE_VX_KK
            && opcode.id != OPCODE_SE_VX_VY
            && opcode.id != OPCODE_SNE_VX_VY
            && opcode.id != OPCODE_SKP_VX
            && opcode.id != OPCODE_SKNP_VX;
    }

    return done;
}
