#pragma once
#include "translate.h"
#include "../chip8.h"

typedef struct {

    CodeCache* caches[4096];

} RecompilerState;

Chip8Error recompiler_init(RecompilerState* repository);
Chip8Error recompiler_step(RecompilerState* repository, Chip8 *state);

