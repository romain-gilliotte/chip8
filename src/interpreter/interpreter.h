#pragma once
#include "../chip8.h"

int chip8_run(Chip8 *state, int microtime);
int chip8_step(Chip8 *state);
