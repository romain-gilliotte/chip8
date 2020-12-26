#include "vm.h"

Chip8Error chip8vm_init(Chip8VirtualMachine* vm, Chip8VirtualMachineType type, Chip8Variant variant, uint32_t clock_speed) {
    vm->type = type;
    
    chip8_init(&vm->state, variant, clock_speed);
    if (type == RECOMPILER)
        recompiler_init(&vm->vm_state.recompiler);

    return CHIP8_OK;
}

Chip8Error chip8vm_load_rom(Chip8VirtualMachine* vm, const char *rom) {
    return chip8_load_rom(&vm->state, rom);
}

Chip8Error chip8vm_run(Chip8VirtualMachine* vm, uint32_t ticks) {
    uint64_t cycles = ticks * vm->state.clock_speed / 1000;
    Chip8Error error = CHIP8_OK;

    while (error == CHIP8_OK && vm->state.cycles_since_started < cycles) {
        error = chip8vm_step(vm);
    }

    return error;
}

Chip8Error chip8vm_step(Chip8VirtualMachine* vm) {
    // Save elapsed cycles, to compute timers later on.
    int32_t cycles_before = vm->state.cycles_since_started;

    // Run virtual machine.
    Chip8Error error;
    if (vm->type == INTERPRETER){
        error = interpreter_step(&vm->state);
    }
    else if (vm->type == RECOMPILER) {
        error = recompiler_step(&vm->vm_state.recompiler, &vm->state);

        // Fallback to interpreter for non supported opcodes.
        if (error == CHIP8_OPCODE_NOT_SUPPORTED)
            error = interpreter_step(&vm->state);
    }

    // Decrement timer at 60Hz, regardless of emulation clock speed.
    int missed_timers =
        + vm->state.cycles_since_started * 60 / vm->state.clock_speed // expected timers
        - cycles_before * 60 / vm->state.clock_speed; // already done timers

    vm->state.DT = vm->state.DT < missed_timers ? 0 : vm->state.DT - missed_timers;
    vm->state.ST = vm->state.ST < missed_timers ? 0 : vm->state.ST - missed_timers;

    return error;
}
