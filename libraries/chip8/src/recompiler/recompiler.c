#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "recompiler.h"

Chip8Error recompiler_init(RecompilerState* repository) {
    memset(repository, 0, sizeof *repository);

    return CHIP8_OK;
}

Chip8Error recompiler_step(RecompilerState* repository, Chip8 *state) {
    // Compile code of the required section if needed.
    CodeCache* cache = repository->caches[state->PC];
    if (!cache) {
        cache = (CodeCache*) malloc(sizeof(CodeCache));
        translate_block(cache, state);
        repository->caches[state->PC] = cache;
    }

    // Run section
    return (Chip8Error) x64_run(&cache->code);
}
