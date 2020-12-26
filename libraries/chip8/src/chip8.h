#pragma once
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>


/**
 * Errors codes.
 */
typedef enum {
    CHIP8_OK = 0,
    CHIP8_ROM_NOT_FOUND = -1,
    CHIP8_ROM_TOO_LONG = -2,
    CHIP8_OPCODE_INVALID = -3,
    CHIP8_OPCODE_NOT_SUPPORTED = -4,
    CHIP8_CALL_STACK_EMPTY = -5,
    CHIP8_CALL_STACK_FULL = -6,
    CHIP8_EXIT = -7,
} Chip8Error;


/**
 * List of supported chip8 variants.
 * 
 * @see https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Extensions-Reference
 */
typedef enum {
    VARIANT_CHIP8,          // http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
    VARIANT_CHIP8_2PAGE,    // https://chip-8.github.io/extensions/#two-page-display-for-chip-8
    VARIANT_SUPER_CHIP,     // https://github.com/JohnEarnest/Octo/blob/gh-pages/docs/SuperChip.md
    VARIANT_XO_CHIP,        // https://github.com/JohnEarnest/Octo/blob/gh-pages/docs/XO-ChipSpecification.md
} Chip8Variant;


typedef struct
{
    Chip8Variant variant;

    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t clock_speed;
    uint32_t cycles_since_started;

    bool display_dirty;

    ////////////
    // Machine
    ////////////

    uint8_t* memory;

    // IO
    bool *display; // one byte per pixel
    uint8_t keyboard[16];

    // Registers
    uint8_t registers[16];
    uint8_t DT;
    uint8_t ST;

    // Pseudo-registers
    uint16_t I;
    uint16_t PC;

    // Stack
    uint8_t SP;
    uint16_t stack[16];

} Chip8;


typedef enum {
    OPCODE_INVALID,

    // Original
    OPCODE_CLS,
    OPCODE_RET,
    OPCODE_JMP_NNN,
    OPCODE_CALL_NNN,
    OPCODE_SE_VX_KK,
    OPCODE_SNE_VX_KK,
    OPCODE_SE_VX_VY,
    OPCODE_LD_VX_KK,
    OPCODE_ADD_VX_KK,
    OPCODE_LD_VX_VY,
    OPCODE_OR_VX_VY,
    OPCODE_AND_VX_VY,
    OPCODE_XOR_VX_VY,
    OPCODE_ADD_VX_VY,
    OPCODE_SUB_VX_VY,
    OPCODE_SHR_VX_VY,
    OPCODE_SUBN_VX_VY,
    OPCODE_SHL_VX_VY,
    OPCODE_SNE_VX_VY,
    OPCODE_LD_I_NNN,
    OPCODE_JP_V0_NNN,
    OPCODE_RND_VX_KK,
    OPCODE_DRW_VX_VY_N,
    OPCODE_SKP_VX,
    OPCODE_SKNP_VX,
    OPCODE_LD_VX_DT,
    OPCODE_LD_VX_K,
    OPCODE_LD_DT_VX,
    OPCODE_LD_ST_VX,
    OPCODE_ADD_I_VX,
    OPCODE_LD_F_VX,
    OPCODE_LD_B_VX,
    OPCODE_LD_I_VX,
    OPCODE_LD_VX_I,

    // Two-page display for CHIP-8
    OPCODE_CLS_HIRES,

    // S-Chip
    OPCODE_SCRL_DOWN_N, // 00cn Scroll display N lines down
    OPCODE_SCRL_LEFT,   // 00fb Scroll display 4 pixels right
    OPCODE_SCRL_RIGHT,  // 00fc Scroll display 4 pixels left
    OPCODE_EXIT,        // 00fd Exit CHIP interpreter
    OPCODE_HIDEF_OFF,   // 00fe Disable extended screen mode
    OPCODE_HIDEF_ON,    // 00ff Enable extended screen mode for full-screen graphics
    OPCODE_LD_I_,       // fx30 Point I to 10-byte font sprite for digit VX (0..9)
    OPCODE_LD_RPL_VX,   // fx75 Store V0..VX in RPL user flags (X <= 7)
    OPCODE_LD_VX_RPL,   // fx85 Read V0..VX from RPL user flags (X <= 7)

    // XO-Chip
    OPCODE_LD_I_VX_VY,  // 5XY2: Save VX..VY to memory starting at I (same as CHIP-8E); order can be ascending or descending; does not increment I
    OPCODE_LD_VX_VY_I,  // 5XY3: Load VX..VY from memory starting at I (same as CHIP-8E); order can be ascending or descending; does not increment I
    OPCODE_LD_I_NNNN,   // F000 NNNN: Load I with 16-bit address NNNN
    OPCODE_DRW_PLN_N,   // FN01: Select drawing planes by bitmask (0 planes, plane 1, plane 2 or both planes (3))
    OPCODE_LD_AUDIO_I,  // F002: Store 16 bytes in audio pattern buffer, starting at I, to be played by the sound buzzer
    OPCODE_SCRL_UP_N,   // 00DN: Scroll up N pixels
} Chip8OpcodeId;


typedef struct {
    Chip8OpcodeId id;
    uint16_t opcode;
    uint8_t x;
    uint8_t y;
    uint8_t n;
    uint8_t kk;
    uint16_t nnn;
} Chip8Opcode;


/**
 * Initialize the Chip8 struct
 * 
 * @param state Pointer to a newly allocated Chip8.
 * @param variant Desired variant.
 * @returns 
 */
Chip8Error chip8_init(Chip8 *state, Chip8Variant variant, uint32_t clock_speed);
Chip8Error chip8_load_rom(Chip8 *state, const char *rom);
Chip8Error chip8_decode(Chip8 *state, Chip8Opcode* opcode, uint16_t address);

int chip8_dump(Chip8 *state, FILE *f);
int chip8_restore(Chip8 *state, FILE *f);
