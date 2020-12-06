#include <stdlib.h>
#include "cache.h"
#include "../chip8.h"
#include "../interpreter/interpreter.h"

int jit_run(CodeCacheRepository* repository, Chip8 *state, uint64_t microtime) {
    uint64_t expected_cc = microtime * state->clock_speed / 1e6;

    while (state->cycle_counts < expected_cc)
    {
        CodeCache* cache = repository->caches[state->PC];

        // Compile code of the required section if needed.
        if (!cache) {
            cache = (CodeCache*) malloc(sizeof(CodeCache));
            create_cache(state, cache);
            repository->caches[state->PC] = cache;
        }

        // Run section.
        int error = jitfn_run(&cache->code);
        if (error < 0) {
            if (error == USE_INTERPRETER) 
                interpreter_step(state);
            else
                return -1;
        }
    }

    return 0;
}
