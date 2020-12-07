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


typedef enum {
    USE_INTERPRETER = -1,

    // return codes 0 and above are the new jump location
    
} CacheReturnCode;

int recompiler_run(CodeCacheRepository* repository, Chip8 *state, uint64_t ticks);
int cache_create(Chip8* state, CodeCache* cache);
