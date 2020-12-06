#pragma once
#include "../chip8.h"

int interpreter_run(Chip8 *state, uint64_t microtime);
int interpreter_step(Chip8 *state);
