#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "recompiler.h"
#include "../chip8.h"
#include "../interpreter/interpreter.h"

static void encode_error(CodeCache* cache, Chip8* state, Chip8Error code) {
    uint32_t ecx_pc = (uint64_t) &state->PC - (uint64_t) state;

    x86_mov_regimm32(&cache->code, EAX, cache->end); // mov eax, cache->end
    x86_mov_memreg16(&cache->code, ECX, ecx_pc, EAX); // mov [state->pc], al

    // return error code, so that the engine can fallback to the interpreter.
    x86_mov_regimm32(&cache->code, EAX, code);
    x86_retn(&cache->code);
}

static void encode(CodeCache* cache, Chip8* state, uint16_t pc) {
    // Compute offsets to access memory. They need to fit in 32bits or less.
    uint32_t ecx_registers = (uint64_t) &state->registers - (uint64_t) state;
    uint32_t ecx_memory = (uint64_t) &state->memory - (uint64_t) state;
    uint32_t ecx_i = (uint64_t) &state->I - (uint64_t) state;
    uint32_t ecx_dt = (uint64_t) &state->DT - (uint64_t) state;
    uint32_t ecx_st = (uint64_t) &state->ST - (uint64_t) state;
    uint32_t ecx_pc = (uint64_t) &state->PC - (uint64_t) state;
    uint32_t ecx_sp = (uint64_t) &state->SP - (uint64_t) state;

    // Decode opcode
    uint16_t opcode = (state->memory[pc] << 8) | state->memory[pc + 1];
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n1 = opcode >> 12;
    uint8_t n4 = opcode & 0xF;
    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t kk = opcode & 0xFF;

    cache->end = pc;
    cache->cycles = (cache->end - cache->start) / 2;

    // if ((opcode & 0xF0FF) != 0xf01e)
    //     return encode_end(cache, CHIP8_OPCODE_NOT_SUPPORTED);

    switch (n1) {
        case 0x0:
            switch (opcode) {
                case 0x00E0: // CLS
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x00EE: // RET
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID);
            }
        
        case 0x1: // JP addr
            x86_mov_regimm32(&cache->code, EAX, nnn); // mov eax, nnn
            x86_mov_memreg16(&cache->code, ECX, ecx_pc, EAX); // mov [state->PC], al
            x86_mov_regimm32(&cache->code, EAX, CHIP8_OK);
            x86_retn(&cache->code);
            return;

        case 0x2: // CALL addr
            // x86_mov_regmem16(&cache->code, EAX, ECX, ecx_pc);
            // x86_mov_regmem8(&cache->code, EDX, ECX, ecx_sp);
            // this needs either a SIB byte, or hacking with the pointer
            

            // x86_inc_mem8(&cache->code, ECX, ecx_sp);

            // x86_mov_regimm32(&cache->code, EAX, nnn); // mov eax, nnn
            // x86_mov_memreg16(&cache->code, ECX, ecx_pc, EAX); // mov [state->PC], al


            return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

        case 0x3: // SE Vx, byte
            return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

        case 0x4: // SNE Vx, byte
            return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

        case 0x5:
            switch (n4) {
                case 0x0: // SE Vx, Vy
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID);
            }

        case 0x6: // LD Vx, kk
            x86_mov_regimm32(&cache->code, EAX, kk); // mov eax, kk
            x86_mov_memreg8(&cache->code, ECX, ecx_registers + x, EAX); // mov [state->registers + x], al
            return encode(cache, state, pc + 2);
        
        case 0x7: // ADD Vx, kk
            x86_mov_regimm32(&cache->code, EAX, kk); // mov eax, kk
            x86_add_memreg8(&cache->code, ECX, ecx_registers + x, EAX); // add [state->registers + x], al
            return encode(cache, state, pc + 2);

        case 0x8:
            switch (n4) {
                case 0x0: // LD Vx, Vy
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y); // mov al, [state->registers + y]
                    x86_mov_memreg8(&cache->code, ECX, ecx_registers + x, EAX); // mov [state->registers + x], al
                    return encode(cache, state, pc + 2);

                case 0x1: // OR Vx, Vy
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x2: // AND Vx, Vy
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x3: // XOR Vx, Vy
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x4: // ADD Vx, Vy
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + y); // mov al, [state->registers + y]
                    x86_add_memreg8(&cache->code, ECX, ecx_registers + x, EAX); // add [state->registers + x], al
                    return encode(cache, state, pc + 2);

                case 0x5: // SUB Vx, Vy
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x6: // SHR Vx, Vy
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x7: // SUBN Vx, Vy
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0xE: // SHL Vx, Vy
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID);
            }

        case 0x9:
            switch (n4)
            {
                case 0x0: // SNE Vx, Vy
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID);
            }

        case 0xA: // LD I, addr
            x86_mov_regimm32(&cache->code, EAX, nnn);
            x86_mov_memreg16(&cache->code, ECX, ecx_i, EAX);
            return encode(cache, state, pc + 2);

        case 0xB: // JP V0, addr
            return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

        case 0xC: // RND Vx, byte
            return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

        case 0xD: // DRW Vx, Vy, nibble
            return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

        case 0xE:
            switch (kk) {
                case 0x9E: // SKP Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0xA1: // SKNP Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID);
            }

        case 0xF:
            switch (kk) {
                case 0x07: // LD Vx, DT
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_dt); // mov al, [state->dt]
                    x86_mov_memreg8(&cache->code, ECX, ecx_registers + x, EAX); // mov [state->registers + x], al
                    return encode(cache, state, pc + 2);

                case 0x0a: // LD Vx, K
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x15: // LD DT, Vx
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + x); // mov al, [state->registers + x]
                    x86_mov_memreg8(&cache->code, ECX, ecx_dt, EAX); // mov [state->dt], al
                    return encode(cache, state, pc + 2);
                
                case 0x18: // LD ST, Vx
                    x86_mov_regmem8(&cache->code, EAX, ECX, ecx_registers + x); // mov al, [state->registers + x]
                    x86_mov_memreg8(&cache->code, ECX, ecx_st, EAX); // mov [state->st], al
                    return encode(cache, state, pc + 2);

                case 0x1e: // ADD I, Vx
                    x86_movzx_regmem8(&cache->code, EAX, ECX, ecx_registers + x); // mov al, [state->registers + x]
                    x86_add_memreg16(&cache->code, ECX, ecx_i, EAX);
                    return encode(cache, state, pc + 2);

                case 0x29: // LD F, Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x33: // LD B, Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x55: // LD [I], Vx
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                case 0x65: // LD Vx, [I]
                    return encode_error(cache, state, CHIP8_OPCODE_NOT_SUPPORTED);

                default:
                    return encode_error(cache, state, CHIP8_OPCODE_INVALID);
            }

        default:
            return encode_error(cache, state, CHIP8_OPCODE_INVALID);
    }
}

static int cache_create(Chip8* state, CodeCache* cache) {
    cache->start = state->PC;
    cache->end = state->PC;
    cache->draws = false;
    cache->cycles = 0;

    x86_init(&cache->code, 4096);
    x86_mov_regimm64(&cache->code, ECX, (uint64_t) state); // Load pointer to chip8 state
    encode(cache, state, state->PC); // Encode instructions until next jump or error.
    x86_lock(&cache->code);

    for (int i = 0; i < cache->code.buffer_ptr; ++i)
        printf("%02hhX", cache->code.buffer[i]);
    printf("   %d\n", (cache->end - cache->start) / 2);
}

int recompiler_init(CodeCacheRepository* repository) {
    memset(repository, 0, sizeof *repository);
}

Chip8Error recompiler_run(CodeCacheRepository* repository, Chip8 *state, uint64_t ticks) {
    uint32_t expected_cc = ticks * state->clock_speed / 1000;

    while (state->cycle_counts < expected_cc)
    {
        CodeCache* cache = repository->caches[state->PC];

        // Compile code of the required section if needed.
        if (!cache) {
            cache = (CodeCache*) malloc(sizeof(CodeCache));
            cache_create(state, cache);
            repository->caches[state->PC] = cache;
        }

        
        // Run section & handle errors
        Chip8Error error = (Chip8Error) x86_run(&cache->code);
        if (error < 0) {
            if (error == CHIP8_OPCODE_NOT_SUPPORTED)
                error = interpreter_step(state);

            if (error < 0)
                return error;
        }

        // Decrement timer at 60Hz, regardless of emulation clock speed.
        state->cycle_counts += cache->cycles;

        int missed_timers = 
            + state->cycle_counts * 60 / state->clock_speed                    // expected timers
            - (state->cycle_counts - cache->cycles) * 60 / state->clock_speed; // actual timers

        state->DT = state->DT < missed_timers ? 0 : state->DT - missed_timers;
        state->ST = state->ST < missed_timers ? 0 : state->ST - missed_timers;
    }

    return CHIP8_OK;
}
