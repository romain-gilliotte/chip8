#pragma once
#include <inttypes.h>
#include "recompiler/recompiler.h"
#include "interpreter/interpreter.h"

typedef enum {
    INTERPRETER,
    RECOMPILER,
} Chip8VirtualMachineType;

typedef struct {
    Chip8VirtualMachineType type;
    Chip8 state;

    union
    {
        RecompilerState recompiler;
    } vm_state;

} Chip8VirtualMachine;

Chip8Error chip8vm_init(Chip8VirtualMachine* vm, Chip8VirtualMachineType type, Chip8Variant variant, uint32_t clock_speed);
Chip8Error chip8vm_load_rom(Chip8VirtualMachine* vm, const char *rom);
Chip8Error chip8vm_run(Chip8VirtualMachine* vm, uint32_t ticks);
Chip8Error chip8vm_step(Chip8VirtualMachine* vm);
