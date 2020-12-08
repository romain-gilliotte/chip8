#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "recompiler.h"
#include "../chip8.h"
#include "../interpreter/interpreter.h"

static int encode(CodeCache* cache, Chip8* state, uint16_t pc) {
    uint16_t opcode = (state->memory[pc] << 8) | state->memory[pc + 1];
    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t kk = opcode & 0xFF;

    cache->end += 2;
    cache->draws = true;

    if ((opcode & 0xF000) == 0x6000) {
        // LD Vx, kk
        x86_mov_regimm32(&cache->code, EAX, kk); // mov al, kk
        x86_mov_memreg8(&cache->code, ECX, x, EAX); // mov [state->registers + x], al

        return encode(cache, state, pc + 2);
    }
    else if ((opcode & 0xF000) == 0x7000) {
        // ADD Vx, kk
        x86_mov_regmem8(&cache->code, EAX, ECX, x); // mov al, [state->registers + x]
        x86_add_regimm8(&cache->code, EAX, kk);     // add al, kk
        x86_mov_memreg8(&cache->code, ECX, x, EAX); // mov [state->registers + x], al

        return encode(cache, state, pc + 2);
    }
    else if ((opcode & 0xF00F) == 0x8000) {
        // LD Vx, Vy
        x86_mov_regmem8(&cache->code, EAX, ECX, y); // mov al, [state->registers + y]
        x86_mov_memreg8(&cache->code, ECX, x, EAX); // mov [state->registers + x], al

        return encode(cache, state, pc + 2);
    }
    else {
        // return error code, so that the engine can fallback to the interpreter.
        x86_mov_regimm32(&cache->code, EAX, OPCODE_NOT_SUPPORTED);
        x86_retn(&cache->code);
    }
}

static int cache_create(Chip8* state, CodeCache* cache) {
    cache->start = state->PC;
    cache->end = state->PC;
    cache->draws = false;

    // Load pointers to chip8 memory and registers.
    x86_init(&cache->code, 4096);
    x86_mov_regimm64(&cache->code, ECX, (uint64_t) &state->registers);
    // x86_mov_regimm64(&cache->code, EDX, (uint64_t) &state->memory);

    // Recursively encode instructions until next jump.
    encode(cache, state, state->PC);
    x86_lock(&cache->code);

    for (int i = 0; i < cache->code.buffer_ptr; ++i)
        printf("%02hhX", cache->code.buffer[i]);
    printf("   %d\n", (cache->end - cache->start) / 2);
}

int recompiler_init(CodeCacheRepository* repository) {
    memset(repository, 0, sizeof *repository);
}

int recompiler_run(CodeCacheRepository* repository, Chip8 *state, uint64_t ticks) {
    uint64_t expected_cc = ticks * state->clock_speed / 1000;

    while (state->cycle_counts < expected_cc)
    {
        CodeCache* cache = repository->caches[state->PC];

        // Compile code of the required section if needed.
        if (!cache) {
            cache = (CodeCache*) malloc(sizeof(CodeCache));
            cache_create(state, cache);
            repository->caches[state->PC] = cache;
        }

        // Run section.
        int error = x86_run(&cache->code);
        state->PC = cache->end;
        state->cycle_counts += (cache->end - cache->start) / 2;

        if (error == OPCODE_NOT_SUPPORTED) {
            state->PC -= 2; // last opcode was not executed
            interpreter_step(state);
        }
    }

    return 0;
}
