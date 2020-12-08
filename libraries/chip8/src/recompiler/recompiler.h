#pragma once
#include "x86.h"
#include "../chip8.h"

typedef struct {
    X86fn code;

    uint16_t start;
    uint16_t end;
    bool draws;
} CodeCache;

typedef struct {

    CodeCache* caches[4096];

} CodeCacheRepository;


typedef enum {
    OPCODE_NOT_SUPPORTED = -2,
} CacheReturnCode;

int recompiler_init(CodeCacheRepository* repository);
int recompiler_run(CodeCacheRepository* repository, Chip8 *state, uint64_t ticks);
