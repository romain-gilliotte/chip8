#pragma once
#include "x86.h"
#include "../chip8.h"

typedef struct {
    X86fn code;
    uint16_t start;
    uint16_t end;
} CodeCache;

typedef struct {

    CodeCache* caches[4096];

} CodeCacheRepository;

void recompiler_init(CodeCacheRepository* repository);
Chip8Error recompiler_run(CodeCacheRepository* repository, Chip8 *state, uint64_t ticks);
