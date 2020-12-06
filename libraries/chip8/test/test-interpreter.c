#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>

#include <chip8.h>
#include <interpreter/interpreter.h>

static int setup(void **state)
{
    Chip8 *chip = malloc(sizeof(Chip8));
    chip8_init(chip, 64, 48, 500);
    *state = chip;

    return 0;
}

static int teardown(void **state)
{
    free(*state);

    return 0;
}

static Chip8 *load_simple_program(void **state, uint16_t opcode)
{
    Chip8 *chip = *state;

    // Load opcode
    chip->memory[0x200] = (opcode >> 8) & 0xFF;
    chip->memory[0x201] = opcode & 0xFF;

    // JMP 0x202 (0x1202)
    chip->memory[0x202] = 0x12;
    chip->memory[0x203] = 0x02;

    return chip;
}

/** 00E0 - CLS */
static void test_00e0(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x00e0);

    chip->display[43] = 1;

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->display[43], 0);
}

/** 00EE - RET */
static void test_00ee(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x00ee);

    chip->stack[0] = 0x250; // Return address
    chip->SP = 1;           // Stack Pointer

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->SP, 0);     // Stack Pointer
    assert_int_equal(chip->PC, 0x252); // Program Count
}

/** 1nnn - JP addr */
static void test_1nnn(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x1210);

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->PC, 0x210);
}

/** 2nnn - CALL addr */
static void test_2nnn(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x2250);

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->stack[0], 0x200); // Return address
    assert_int_equal(chip->SP, 1);           // Stack Pointer
    assert_int_equal(chip->PC, 0x250);       // Program Count
}

/** 3xkk - SE Vx, byte */
static void test_3xkk_equal(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x3201);

    chip->registers[2] = 0x01;
    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->PC, 0x204);
}

static void test_3xkk_notequal(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x3201);

    chip->registers[2] = 0x10;
    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->PC, 0x202);
}

// /** 4xkk - SNE Vx, byte */
// static void test_4xkk(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x4xkk);
// }

// /** 5xy0 - SE Vx, Vy */
// static void test_5xy0(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x5xy0);
// }

// /** 6xkk - LD Vx, byte */
// static void test_6xkk(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x6xkk);
// }

/** 7xkk - ADD Vx, byte */
static void test_7xkk_simple(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x7210);
    chip->registers[2] = 0x10;

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->registers[2], 0x20);
    assert_int_equal(chip->registers[15], 0x00);
    assert_int_equal(chip->PC, 0x202);
}

/** 7xkk - ADD Vx, byte */
static void test_7xkk_overflow(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x7210);
    chip->registers[2] = 0xff;
    chip->registers[15] = 0x12;

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->registers[2], 0x0f);
    assert_int_equal(chip->registers[15], 0x12); // As per documentation, no carry should be generated
    assert_int_equal(chip->PC, 0x202);
}

/** 8xy0 - LD Vx, Vy */
static void test_8xy0(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x8230);

    chip->registers[3] = 0x10;
    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->registers[2], 0x10);
    assert_int_equal(chip->registers[3], 0x10);
    assert_int_equal(chip->PC, 0x202);
}

// /** 8xy1 - OR Vx, Vy */
// static void test_8xy1(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x8xy1);
// }

/** 8xy2 - AND Vx, Vy */
// static void test_8xy2(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x8xy1);
// }

// /** 8xy3 - XOR Vx, Vy */
// static void test_8xy3(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x8xy3);
// }

/** 8xy4 - ADD Vx, Vy */
static void test_8xy4_simple(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x8124);
    chip->registers[1] = 0x10;
    chip->registers[2] = 0x20;

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->registers[1], 0x30);
    assert_int_equal(chip->registers[2], 0x20);
    assert_int_equal(chip->registers[15], 0x00);
    assert_int_equal(chip->PC, 0x202);
}

static void test_8xy4_overflow(void **state)
{
    Chip8 *chip = load_simple_program(state, 0x8124);
    chip->registers[1] = 0x10;
    chip->registers[2] = 0xff;

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->registers[1], 0x0f);
    assert_int_equal(chip->registers[2], 0xff);
    assert_int_equal(chip->registers[15], 0x01);
    assert_int_equal(chip->PC, 0x202);
}

// /** 8xy5 - SUB Vx, Vy */
// static void test_8xy5(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x8xy5);
// }

// /** 8xy6 - SHR Vx, Vy */
// static void test_8xy6(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x8xy6);
// }

// /** 8xy7 - SUBN Vx, Vy */
// static void test_8xy7(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x8xy7);
// }

// /** 8xyE - SHL Vx, Vy */
// static void test_8xye(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x8xye);
// }

// /** 9xy0 - SNE Vx, Vy */
// static void test_9xy0(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0x9xy0);
// }

/** Annn - LD I, addr */
static void test_annn(void **state)
{
    Chip8 *chip = load_simple_program(state, 0xa21e);

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->I, 0x021e);
    assert_int_equal(chip->PC, 0x202);
}

// /** Bnnn - JP V0, addr */
// static void test_bnnn(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xbnnn);
// }

/** Cxkk - RND Vx, byte */
static void test_cxkk(void **state)
{
    srand(0);

    Chip8 *chip = load_simple_program(state, 0xc2f0);

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->registers[2], 0x60);
    assert_int_equal(chip->PC, 0x202);
}

/** Dxyn - DRW Vx, Vy, nibble */
static void test_dxyn_simple(void **state)
{
    // 3 bytes sprite at position (V0, V1)
    Chip8 *chip = load_simple_program(state, 0xd013);

    chip->registers[0] = 3;
    chip->registers[1] = 3;
    chip->I = 0x0400;
    chip->memory[0x0400] = 12; // binary: 0000 1100
    chip->memory[0x0401] = 34; // binary: 0010 0010
    chip->memory[0x0402] = 56; // binary: 0011 1000

    assert_int_equal(interpreter_step(chip), 0);

    // Test byte one of the sprite
    assert_int_equal(chip->display[64 * 3 + 3], 0);
    assert_int_equal(chip->display[64 * 3 + 4], 0);
    assert_int_equal(chip->display[64 * 3 + 5], 0);
    assert_int_equal(chip->display[64 * 3 + 6], 0);
    assert_int_equal(chip->display[64 * 3 + 7], 1);
    assert_int_equal(chip->display[64 * 3 + 8], 1);
    assert_int_equal(chip->display[64 * 3 + 9], 0);
    assert_int_equal(chip->display[64 * 3 + 10], 0);

    // Test byte three of the sprite
    assert_int_equal(chip->display[64 * 5 + 3], 0);
    assert_int_equal(chip->display[64 * 5 + 4], 0);
    assert_int_equal(chip->display[64 * 5 + 5], 1);
    assert_int_equal(chip->display[64 * 5 + 6], 1);
    assert_int_equal(chip->display[64 * 5 + 7], 1);
    assert_int_equal(chip->display[64 * 5 + 8], 0);
    assert_int_equal(chip->display[64 * 5 + 9], 0);
    assert_int_equal(chip->display[64 * 5 + 10], 0);

    // Test collisions
    assert_int_equal(chip->registers[15], 0);
}

// /** Ex9E - SKP Vx */
// static void test_ex9e(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xex9e);
// }

// /** ExA1 - SKNP Vx */
// static void test_exa1(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xexa1);
// }

// /** Fx07 - LD Vx, DT */
// static void test_fx07(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xfx07);
// }

// /** Fx0A - LD Vx, K */
// static void test_fx0a(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xfx0a);
// }

// /** Fx15 - LD DT, Vx */
// static void test_fx15(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xfx15);
// }

// /** Fx18 - LD ST, Vx */
// static void test_fx18(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xfx18);
// }

// /** Fx1E - ADD I, Vx */
// static void test_fx1e(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xfx1e);
// }

// /** Fx29 - LD F, Vx */
// static void test_fx29(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xfx29);
// }

/** Fx33 - LD B, Vx */
static void test_fx33(void **state)
{
    Chip8 *chip = load_simple_program(state, 0xf233);
    chip->I = 1024; // Can be any number other than 0x200 to avoid overwriting the opcode.
    chip->registers[2] = 127;

    assert_int_equal(interpreter_step(chip), 0);
    assert_int_equal(chip->registers[2], 127);
    assert_int_equal(chip->memory[chip->I], 1);
    assert_int_equal(chip->memory[chip->I + 1], 2);
    assert_int_equal(chip->memory[chip->I + 2], 7);
    assert_int_equal(chip->PC, 0x202);
}

// /** Fx55 - LD [I], Vx */
// static void test_fx55(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xfx55);
// }

// /** Fx65 - LD Vx, [I] */
// static void test_fx65(void **state)
// {
//     Chip8 *chip = load_simple_program(state, 0xfx65);
// }

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_00e0, setup, teardown),
        cmocka_unit_test_setup_teardown(test_00ee, setup, teardown),
        cmocka_unit_test_setup_teardown(test_1nnn, setup, teardown),
        cmocka_unit_test_setup_teardown(test_2nnn, setup, teardown),
        cmocka_unit_test_setup_teardown(test_3xkk_equal, setup, teardown),
        cmocka_unit_test_setup_teardown(test_3xkk_notequal, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_4xkk, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_5xy0, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_6xkk, setup, teardown),
        cmocka_unit_test_setup_teardown(test_7xkk_simple, setup, teardown),
        cmocka_unit_test_setup_teardown(test_7xkk_overflow, setup, teardown),
        cmocka_unit_test_setup_teardown(test_8xy0, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_8xy1, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_8xy2, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_8xy3, setup, teardown),
        cmocka_unit_test_setup_teardown(test_8xy4_simple, setup, teardown),
        cmocka_unit_test_setup_teardown(test_8xy4_overflow, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_8xy5, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_8xy6, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_8xy7, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_8xye, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_9xy0, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_annn, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_bnnn, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cxkk, setup, teardown),
        cmocka_unit_test_setup_teardown(test_dxyn_simple, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_ex9e, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_exa1, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_fx07, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_fx0a, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_fx15, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_fx18, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_fx1e, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_fx29, setup, teardown),
        cmocka_unit_test_setup_teardown(test_fx33, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_fx55, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_fx65, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
