#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "recompiler.h"
#include "../chip8.h"
#include "../interpreter/interpreter.h"

static void cache_create(Chip8* state, CodeCache* cache);
static bool encode_instruction(CodeCache* cache, Chip8* state);
static bool encode_error(CodeCache* cache, Chip8* state, Chip8Error code);
static int  next_length(CodeCache* cache, Chip8* state);
static bool is_skip(uint16_t opcode);


void recompiler_init(CodeCacheRepository* repository) {
    memset(repository, 0, sizeof *repository);
}

Chip8Error recompiler_run(CodeCacheRepository* repository, Chip8 *state, uint64_t ticks) {
    uint32_t expected_cc = ticks * state->clock_speed / 1000;

    while (state->cycles_since_started < expected_cc)
    {
        CodeCache* cache = repository->caches[state->PC];

        // Compile code of the required section if needed.
        if (!cache) {
            cache = (CodeCache*) malloc(sizeof(CodeCache));
            cache_create(state, cache);
            repository->caches[state->PC] = cache;
        }

        // Save elapsed cycles, to compute timers later on.
        int32_t cycles_before = state->cycles_since_started;

        // Run section & handle errors
        Chip8Error error = (Chip8Error) x86_run(&cache->code);
        if (error < 0) {
            if (error == CHIP8_OPCODE_NOT_SUPPORTED) {
                // Replay last instruction in interpreter
                cache->end -= 2;
                error = interpreter_step(state);
            }

            if (error < 0)
                return error;
        }

        // Decrement timer at 60Hz, regardless of emulation clock speed.
        int missed_timers = 
            + state->cycles_since_started * 60 / state->clock_speed // expected timers
            - cycles_before * 60 / state->clock_speed; // already done timers

        state->DT = state->DT < missed_timers ? 0 : state->DT - missed_timers;
        state->ST = state->ST < missed_timers ? 0 : state->ST - missed_timers;
    }

    return CHIP8_OK;
}

static void cache_create(Chip8* state, CodeCache* cache) {
    cache->start = state->PC;
    cache->end = state->PC;

    x86_init(&cache->code, 4096);
    x86_mov_regimm64(&cache->code, ECX, (uint64_t) state); // Load pointer to chip8 state

    while (encode_instruction(cache, state)) {
        cache->end += 2;
    }

    x86_lock(&cache->code);

    // debug print
    for (uint32_t i = 0; i < cache->code.buffer_ptr; ++i)
        printf("%02hhX", cache->code.buffer[i]);
    printf("   %d\n", (cache->end - cache->start) / 2);
}


static bool encode_instruction(CodeCache* cache, Chip8* state) {
    // Compute offsets to access memory. They need to fit in 32bits or less.
    uint32_t ecx_registers = (uint64_t) &state->registers - (uint64_t) state;
    uint32_t ecx_memory = (uint64_t) &state->memory - (uint64_t) state;
    uint32_t ecx_i = (uint64_t) &state->I - (uint64_t) state;
    uint32_t ecx_dt = (uint64_t) &state->DT - (uint64_t) state;
    uint32_t ecx_st = (uint64_t) &state->ST - (uint64_t) state;
    uint32_t ecx_pc = (uint64_t) &state->PC - (uint64_t) state;
    uint32_t ecx_sp = (uint64_t) &state->SP - (uint64_t) state;
    uint32_t ecx_cycles = (uint64_t) &state->cycles_since_started - (uint64_t) state;

    // Decode opcode
    uint16_t opcode = (state->memory[cache->end] << 8) | state->memory[cache->end + 1];
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n1 = opcode >> 12;
    uint8_t n4 = opcode & 0xF;
    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t kk = opcode & 0xFF;

    // Special case: Instruction just after an SE cannot be the end of the block
    bool must_continue = false;
    if (cache->end >= 2 && cache->start < cache->end)
        must_continue = is_skip((state->memory[cache->end - 2] << 8) | state->memory[cache->end - 1]);

    // if ((opcode & 0x8000) != 0x8000) {
    //                 return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;
    // }

    switch (n1) {
        case 0x0:
            switch (opcode) {
                case 0x00E0: // CLS
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

                case 0x00EE: // RET
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID) || must_continue;
            }

        case 0x1: // JP addr
            // Update PC and cycles
            x86_mov_regimm32(&cache->code, EAX, nnn);
            x86_mov_memreg16(&cache->code, ECX, ecx_pc, EAX);
            x86_mov_regimm32(&cache->code, EAX, 1 + (cache->end - cache->start) / 2);
            x86_add_memreg32(&cache->code, ECX, ecx_cycles, EAX);

            // return CHIP8_OK
            x86_mov_regimm32(&cache->code, EAX, CHIP8_OK);
            x86_retn(&cache->code);
            return must_continue;

        case 0x2: // CALL addr
            return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

        case 0x3: // SE Vx, byte
            x86_dec_mem32(&cache->code, ECX, ecx_cycles); // cycles--
            x86_mov_regimm32(&cache->code, EAX, kk); // load kk in register
            x86_cmp_regmem8(&cache->code, EAX, ECX, ecx_registers + x); // cmp Vx, kk
            x86_jz8(&cache->code, 3 + next_length(cache, state)); // jz after next instruction (inc is 3bytes)
            x86_inc_mem32(&cache->code, ECX, ecx_cycles); // cycles++
            return true;

        case 0x4: // SNE Vx, byte
            x86_dec_mem32(&cache->code, ECX, ecx_cycles); // cycles--
            x86_mov_regimm32(&cache->code, EAX, kk); // load kk in register
            x86_cmp_regmem8(&cache->code, EAX, ECX, ecx_registers + x); // cmp Vx, kk
            x86_jnz8(&cache->code, 3 + next_length(cache, state)); // jz after next instruction (inc is 3bytes)
            x86_inc_mem32(&cache->code, ECX, ecx_cycles); // cycles++
            return true;

        case 0x5:
            switch (n4) {
                case 0x0: // SE Vx, Vy
                    x86_dec_mem32(&cache->code, ECX, ecx_cycles); // cycles--
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y); // mov al, [state->registers + y]
                    x86_cmp_regmem8(&cache->code, EAX, ECX, ecx_registers + x); // cmp Vx, kk
                    x86_jz8(&cache->code, 3 + next_length(cache, state)); // jz after next instruction (inc is 3bytes)
                    x86_inc_mem32(&cache->code, ECX, ecx_cycles); // cycles++
                    return true;

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID) || must_continue;
            }

        case 0x6: // LD Vx, kk
            x86_mov_regimm32(&cache->code, EAX, kk); // mov eax, kk
            x86_mov_memreg8(&cache->code, ECX, ecx_registers + x, EAX); // mov [state->registers + x], al
            return true;
        
        case 0x7: // ADD Vx, kk
            x86_mov_regimm32(&cache->code, EAX, kk); // mov eax, kk
            x86_add_memreg8(&cache->code, ECX, ecx_registers + x, EAX); // add [state->registers + x], al
            return true;

        case 0x8:
            switch (n4) {
                case 0x0: // LD Vx, Vy
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y);
                    x86_mov_memreg8(&cache->code, ECX, ecx_registers + x, EAX);
                    return true;

                case 0x1: // OR Vx, Vy
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y);
                    x86_or_memreg8(&cache->code, ECX, ecx_registers + x, EAX);
                    return true;

                case 0x2: // AND Vx, Vy
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y);
                    x86_and_memreg8(&cache->code, ECX, ecx_registers + x, EAX);
                    return true;

                case 0x3: // XOR Vx, Vy
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y);
                    x86_xor_memreg8(&cache->code, ECX, ecx_registers + x, EAX);
                    return true;

                case 0x4: // ADD Vx, Vy
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y);
                    x86_add_memreg8(&cache->code, ECX, ecx_registers + x, EAX);
                    x86_setc(&cache->code, ECX, ecx_registers + 15);
                    return true;

                case 0x5: // SUB Vx, Vy
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y);
                    x86_sub_memreg8(&cache->code, ECX, ecx_registers + x, EAX);
                    x86_setnc(&cache->code, ECX, ecx_registers + 15); // x > y
                    return true;

                case 0x6: // SHR Vx, Vy
                    x86_shr_memreg8(&cache->code, ECX, ecx_registers + x);
                    x86_setc(&cache->code, ECX, ecx_registers + 15);
                    return true;

                case 0x7: // SUBN Vx, Vy
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y);
                    x86_sub_regmem8(&cache->code, EAX, ECX, ecx_registers + x);
                    x86_setnc(&cache->code, ECX, ecx_registers + 15);
                    x86_mov_memreg8(&cache->code, ECX, ecx_registers + x, EAX);
                    return true;

                case 0xE: // SHL Vx, Vy
                    x86_shl_memreg8(&cache->code, ECX, ecx_registers + x);
                    x86_setc(&cache->code, ECX, ecx_registers + 15);
                    return true;

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID) || must_continue;
            }

        case 0x9:
            switch (n4)
            {
                case 0x0: // SNE Vx, Vy
                    x86_dec_mem32(&cache->code, ECX, ecx_cycles); // cycles--
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y); // mov al, [state->registers + y]
                    x86_cmp_regmem8(&cache->code, EAX, ECX, ecx_registers + x); // cmp Vx, kk
                    x86_jnz8(&cache->code, 3 + next_length(cache, state)); // jz after next instruction (inc is 3bytes)
                    x86_inc_mem32(&cache->code, ECX, ecx_cycles); // cycles++
                    return true;

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID) || must_continue;
            }

        case 0xA: // LD I, addr
            x86_mov_regimm32(&cache->code, EAX, nnn);
            x86_mov_memreg16(&cache->code, ECX, ecx_i, EAX);
            return true;

        case 0xB: // JP V0, addr
            // Update PC and cycles
            x86_mov_regimm32(&cache->code, EAX, nnn);
            x86_mov_memreg16(&cache->code, ECX, ecx_pc, EAX);
            x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers); // not super efficient
            x86_add_memreg16(&cache->code, ECX, ecx_pc, EAX);

            // Update cycles
            x86_mov_regimm32(&cache->code, EAX, 1 + (cache->end - cache->start) / 2);
            x86_add_memreg32(&cache->code, ECX, ecx_cycles, EAX);

            // return CHIP8_OK
            x86_mov_regimm32(&cache->code, EAX, CHIP8_OK);
            x86_retn(&cache->code);
            return must_continue;

        case 0xC: // RND Vx, byte
            return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

        case 0xD: // DRW Vx, Vy, nibble
            return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

        case 0xE:
            switch (kk) {
                case 0x9E: // SKP Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

                case 0xA1: // SKNP Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID) || must_continue;
            }

        case 0xF:
            switch (kk) {
                case 0x07: // LD Vx, DT
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_dt);
                    x86_mov_memreg8(&cache->code, ECX, ecx_registers + x, EAX);
                    return true;

                case 0x0a: // LD Vx, K
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

                case 0x15: // LD DT, Vx
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + x);
                    x86_mov_memreg8(&cache->code, ECX, ecx_dt, EAX);
                    return true;
                
                case 0x18: // LD ST, Vx
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + x);
                    x86_mov_memreg8(&cache->code, ECX, ecx_st, EAX);
                    return true;

                case 0x1e: // ADD I, Vx
                    x86_movzx_regmem8(&cache->code, EAX, ECX, ecx_registers + x);
                    x86_add_memreg16(&cache->code, ECX, ecx_i, EAX);
                    return true;

                case 0x29: // LD F, Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

                case 0x33: // LD B, Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

                case 0x55: // LD [I], Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

                case 0x65: // LD Vx, [I]
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED) || must_continue;

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID) || must_continue;
            }

        default:
            return encode_error(cache, state, CHIP8_OPCODE_INVALID) || must_continue;
    }
}

/**
 * Encode asm sequence for errors (invalid/unsupported opcode).
 * This simply make the generated function update the state (PC & elapsed cycles)
 * and return the error to the caller.
 */
static bool encode_error(CodeCache* cache, Chip8* state, Chip8Error code) {
    if (cache->start < cache->end) {
        // Update program counter.
        uint32_t ecx_pc = (uint64_t) &state->PC - (uint64_t) state;
        x86_mov_regimm32(&cache->code, EAX, cache->end);
        x86_mov_memreg16(&cache->code, ECX, ecx_pc, EAX);

        // Update elapsed cycles
        uint32_t ecx_cycles = (uint64_t) &state->cycles_since_started - (uint64_t) state;
        x86_mov_regimm32(&cache->code, EAX, (cache->end - cache->start) / 2);
        x86_add_memreg32(&cache->code, ECX, ecx_cycles, EAX);
    }

    // return error code.
    x86_mov_regimm32(&cache->code, EAX, code);
    x86_retn(&cache->code);

    return false;
}

/** Compute length of next instruction */
static int next_length(CodeCache* cache, Chip8* state) {
    cache->end += 2;
    
    int buffer_ptr = cache->code.buffer_ptr; // Save pointer
    encode_instruction(cache, state); // Write instruction

    int length = cache->code.buffer_ptr - buffer_ptr; // compute length
    
    // Restore cache.
    cache->code.buffer_ptr = buffer_ptr;
    cache->end -= 2;
    return length;
}

/** Is this opcode a skip instruction? */
static bool is_skip(uint16_t opcode) {
    return (opcode & 0xF000) == 0x3000  // SE   Vx, kk
        || (opcode & 0xF000) == 0x4000  // SNE  Vx, kk
        || (opcode & 0xF00F) == 0x5000  // SE   Vx, Vy
        || (opcode & 0xF00F) == 0x9000  // SNE  Vx, Vy
        || (opcode & 0xF0FF) == 0xE09E  // SKP  Vx
        || (opcode & 0xF0FF) == 0xE0A1; // SKNP Vx
}