#pragma once
#include "x64.h"
#include "../chip8.h"

typedef struct {
    X86fn code;
    uint16_t start;
    uint16_t end;
} CodeCache;

void translate_block(CodeCache* cache, Chip8* state);

/**
 * Reads the instruction which is at cache->end in the Chip8 memory and translate it in x64 code.
 * This function assumes that a pointer to the Chip8 state was previously loaded in the ECX register.
 * Note: cache->end is NOT incremented
 * 
 * @returns true when the block is finished, false otherwise
 */
bool translate_instruction(CodeCache* cache, Chip8* state);
