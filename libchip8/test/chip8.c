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

static Chip8 *set_opcode(void **state, uint16_t opcode)
{
    Chip8 *chip = *state;
    return chip;
}

/** 0nnn - SYS addr */
static void test_0nnn(void **state)
{
    Chip8 *chip = set_opcode(state, 0x0nnn);
}

/** 00E0 - CLS */
static void test_00e0(void **state)
{
    Chip8 *chip = set_opcode(state, 0x00e0);
}

/** 00EE - RET */
static void test_00ee(void **state)
{
    Chip8 *chip = set_opcode(state, 0x00ee);
}

/** 1nnn - JP addr */
static void test_1nnn(void **state)
{
    Chip8 *chip = set_opcode(state, 0x1nnn);
}

/** 2nnn - CALL addr */
static void test_2nnn(void **state)
{
    Chip8 *chip = set_opcode(state, 0x2nnn);
}

/** 3xkk - SE Vx, byte */
static void test_3xkk(void **state)
{
    Chip8 *chip = set_opcode(state, 0x3xkk);
}

/** 4xkk - SNE Vx, byte */
static void test_4xkk(void **state)
{
    Chip8 *chip = set_opcode(state, 0x4xkk);
}

/** 5xy0 - SE Vx, Vy */
static void test_5xy0(void **state)
{
    Chip8 *chip = set_opcode(state, 0x5xy0);
}

/** 6xkk - LD Vx, byte */
static void test_6xkk(void **state)
{
    Chip8 *chip = set_opcode(state, 0x6xkk);
}

/** 7xkk - ADD Vx, byte */
static void test_7xkk(void **state)
{
    Chip8 *chip = set_opcode(state, 0x7xkk);
}

/** 8xy0 - LD Vx, Vy */
static void test_8xy0(void **state)
{
    Chip8 *chip = set_opcode(state, 0x8xy0);
}

/** 8xy1 - OR Vx, Vy */
static void test_8xy1(void **state)
{
    Chip8 *chip = set_opcode(state, 0x8xy1);
}

/** 8xy2 - AND Vx, Vy */
static void test_8xy2(void **state)
{
    Chip8 *chip = set_opcode(state, 0x8xy2);
}

/** 8xy3 - XOR Vx, Vy */
static void test_8xy3(void **state)
{
    Chip8 *chip = set_opcode(state, 0x8xy3);
}

/** 8xy4 - ADD Vx, Vy */
static void test_8xy4(void **state)
{
    Chip8 *chip = set_opcode(state, 0x8xy4);
}

/** 8xy5 - SUB Vx, Vy */
static void test_8xy5(void **state)
{
    Chip8 *chip = set_opcode(state, 0x8xy5);
}

/** 8xy6 - SHR Vx, Vy */
static void test_8xy6(void **state)
{
    Chip8 *chip = set_opcode(state, 0x8xy6);
}

/** 8xy7 - SUBN Vx, Vy */
static void test_8xy7(void **state)
{
    Chip8 *chip = set_opcode(state, 0x8xy7);
}

/** 8xyE - SHL Vx, Vy */
static void test_8xye(void **state)
{
    Chip8 *chip = set_opcode(state, 0x8xye);
}

/** 9xy0 - SNE Vx, Vy */
static void test_9xy0(void **state)
{
    Chip8 *chip = set_opcode(state, 0x9xy0);
}

/** Annn - LD I, addr */
static void test_annn(void **state)
{
    Chip8 *chip = set_opcode(state, 0xannn);
}

/** Bnnn - JP V0, addr */
static void test_bnnn(void **state)
{
    Chip8 *chip = set_opcode(state, 0xbnnn);
}

/** Cxkk - RND Vx, byte */
static void test_cxkk(void **state)
{
    Chip8 *chip = set_opcode(state, 0xcxkk);
}

/** Dxyn - DRW Vx, Vy, nibble */
static void test_dxyn(void **state)
{
    Chip8 *chip = set_opcode(state, 0xdxyn);
}

/** Ex9E - SKP Vx */
static void test_ex9e(void **state)
{
    Chip8 *chip = set_opcode(state, 0xex9e);
}

/** ExA1 - SKNP Vx */
static void test_exa1(void **state)
{
    Chip8 *chip = set_opcode(state, 0xexa1);
}

/** Fx07 - LD Vx, DT */
static void test_fx07(void **state)
{
    Chip8 *chip = set_opcode(state, 0xfx07);
}

/** Fx0A - LD Vx, K */
static void test_fx0a(void **state)
{
    Chip8 *chip = set_opcode(state, 0xfx0a);
}

/** Fx15 - LD DT, Vx */
static void test_fx15(void **state)
{
    Chip8 *chip = set_opcode(state, 0xfx15);
}

/** Fx18 - LD ST, Vx */
static void test_fx18(void **state)
{
    Chip8 *chip = set_opcode(state, 0xfx18);
}

/** Fx1E - ADD I, Vx */
static void test_fx1e(void **state)
{
    Chip8 *chip = set_opcode(state, 0xfx1e);
}

/** Fx29 - LD F, Vx */
static void test_fx29(void **state)
{
    Chip8 *chip = set_opcode(state, 0xfx29);
}

/** Fx33 - LD B, Vx */
static void test_fx33(void **state)
{
    Chip8 *chip = set_opcode(state, 0xfx33);
}

/** Fx55 - LD [I], Vx */
static void test_fx55(void **state)
{
    Chip8 *chip = set_opcode(state, 0xfx55);
}

/** Fx65 - LD Vx, [I] */
static void test_fx65(void **state)
{
    Chip8 *chip = set_opcode(state, 0xfx65);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_0nnn),
        cmocka_unit_test(test_00e0),
        cmocka_unit_test(test_00ee),
        cmocka_unit_test(test_1nnn),
        cmocka_unit_test(test_2nnn),
        cmocka_unit_test(test_3xkk),
        cmocka_unit_test(test_4xkk),
        cmocka_unit_test(test_5xy0),
        cmocka_unit_test(test_6xkk),
        cmocka_unit_test(test_7xkk),
        cmocka_unit_test(test_8xy0),
        cmocka_unit_test(test_8xy1),
        cmocka_unit_test(test_8xy2),
        cmocka_unit_test(test_8xy3),
        cmocka_unit_test(test_8xy4),
        cmocka_unit_test(test_8xy5),
        cmocka_unit_test(test_8xy6),
        cmocka_unit_test(test_8xy7),
        cmocka_unit_test(test_8xye),
        cmocka_unit_test(test_9xy0),
        cmocka_unit_test(test_annn),
        cmocka_unit_test(test_bnnn),
        cmocka_unit_test(test_cxkk),
        cmocka_unit_test(test_dxyn),
        cmocka_unit_test(test_ex9e),
        cmocka_unit_test(test_exa1),
        cmocka_unit_test(test_fx07),
        cmocka_unit_test(test_fx0a),
        cmocka_unit_test(test_fx15),
        cmocka_unit_test(test_fx18),
        cmocka_unit_test(test_fx1e),
        cmocka_unit_test(test_fx29),
        cmocka_unit_test(test_fx33),
        cmocka_unit_test(test_fx55),
        cmocka_unit_test(test_fx65),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
