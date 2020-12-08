#pragma once
#include "../chip8.h"

Chip8Error interpreter_run(Chip8 *state, uint32_t ticks);
Chip8Error interpreter_step(Chip8 *state);
