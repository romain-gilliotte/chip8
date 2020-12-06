#pragma once
#include "jitfn.h"
#include "../chip8.h"

typedef struct {
    JitFn code;

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

int create_cache(Chip8* state, CodeCache* cache);
