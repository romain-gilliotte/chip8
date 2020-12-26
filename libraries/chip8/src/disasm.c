#include "chip8.h"

static void disassemble_invalid(Chip8Opcode* opcode, FILE* f) {
    (void) opcode;
    fprintf(f, "Invalid Opcode 0x%04x", opcode->opcode);
}

static void disassemble_cls(Chip8Opcode* opcode, FILE* f) {
    (void) opcode;
    fprintf(f, "CLS");
}

static void disassemble_ret(Chip8Opcode* opcode, FILE* f) {
    (void) opcode;
    fprintf(f, "RET");
}

static void disassemble_jmp_nnn(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "JMP  0x%04x", opcode->nnn);
}

static void disassemble_call_nnn(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "CALL 0x%04x", opcode->nnn);
}

static void disassemble_se_vx_kk(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SE   V%x, 0x%02x", opcode->x, opcode->kk);
}

static void disassemble_sne_vx_kk(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SNE  V%x, 0x%02x", opcode->x, opcode->kk);
}

static void disassemble_se_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SE   V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_ld_vx_kk(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   V%x, 0x%02x", opcode->x, opcode->kk);
}

static void disassemble_add_vx_kk(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "ADD  V%x, 0x%02x", opcode->x, opcode->kk);
}

static void disassemble_ld_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_or_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "OR   V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_and_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "AND  V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_xor_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "XOR  V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_add_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "ADD  V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_sub_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SUB  V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_shr_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SHR  V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_subn_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SUBN V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_shl_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SHL  V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_sne_vx_vy(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SNE  V%x, V%x", opcode->x, opcode->y);
}

static void disassemble_ld_i_nnn(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   I,  0x%04x", opcode->nnn);
}

static void disassemble_jp_v0_nnn(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "JP   V0, 0x%04x", opcode->nnn);
}

static void disassemble_rnd_vx_kk(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "RND  V%x, 0x%02x", opcode->x, opcode->kk);
}

static void disassemble_drw_vx_vy_n(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "DRW  V%x, V%x, %d", opcode->x, opcode->y, opcode->n);
}

static void disassemble_skp_vx(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SKP  V%x", opcode->x);
}

static void disassemble_sknp_vx(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "SKNP V%x", opcode->x);
}

static void disassemble_ld_vx_dt(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   V%x, DT", opcode->x);
}

static void disassemble_ld_vx_k(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   V%x, K", opcode->x);
}

static void disassemble_ld_dt_vx(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   DT, V%x", opcode->x);
}

static void disassemble_ld_st_vx(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   ST, V%x", opcode->x);
}

static void disassemble_add_i_vx(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "ADD  I, V%x", opcode->x);
}

static void disassemble_ld_f_vx(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   F, V%x", opcode->x);
}

static void disassemble_ld_b_vx(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   B, V%x", opcode->x);
}

static void disassemble_ld_i_vx(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   [I], V%x", opcode->x);
}

static void disassemble_ld_vx_i(Chip8Opcode* opcode, FILE* f) {
    fprintf(f, "LD   V%x, [I]", opcode->x);
}

static void (*disassemble_arr[])(Chip8Opcode*, FILE*) = {
    // Original
    disassemble_invalid,       // OPCODE_INVALID
    disassemble_cls,           // OPCODE_CLS,
    disassemble_ret,           // OPCODE_RET,
    disassemble_jmp_nnn,       // OPCODE_JMP_NNN,
    disassemble_call_nnn,      // OPCODE_CALL_NNN,
    disassemble_se_vx_kk,      // OPCODE_SE_VX_KK,
    disassemble_sne_vx_kk,     // OPCODE_SNE_VX_KK,
    disassemble_se_vx_vy,      // OPCODE_SE_VX_VY,
    disassemble_ld_vx_kk,      // OPCODE_LD_VX_KK,
    disassemble_add_vx_kk,     // OPCODE_ADD_VX_KK,
    disassemble_ld_vx_vy,      // OPCODE_LD_VX_VY,
    disassemble_or_vx_vy,      // OPCODE_OR_VX_VY,
    disassemble_and_vx_vy,     // OPCODE_AND_VX_VY,
    disassemble_xor_vx_vy,     // OPCODE_XOR_VX_VY,
    disassemble_add_vx_vy,     // OPCODE_ADD_VX_VY,
    disassemble_sub_vx_vy,     // OPCODE_SUB_VX_VY,
    disassemble_shr_vx_vy,     // OPCODE_SHR_VX_VY,
    disassemble_subn_vx_vy,    // OPCODE_SUBN_VX_VY,
    disassemble_shl_vx_vy,     // OPCODE_SHL_VX_VY,
    disassemble_sne_vx_vy,     // OPCODE_SNE_VX_VY,
    disassemble_ld_i_nnn,      // OPCODE_LD_I_NNN,
    disassemble_jp_v0_nnn,     // OPCODE_JP_V0_NNN,
    disassemble_rnd_vx_kk,     // OPCODE_RND_VX_KK,
    disassemble_drw_vx_vy_n,   // OPCODE_DRW_VX_VY_N,
    disassemble_skp_vx,        // OPCODE_SKP_VX,
    disassemble_sknp_vx,       // OPCODE_SKNP_VX,
    disassemble_ld_vx_dt,      // OPCODE_LD_VX_DT,
    disassemble_ld_vx_k,       // OPCODE_LD_VX_K,
    disassemble_ld_dt_vx,      // OPCODE_LD_DT_VX,
    disassemble_ld_st_vx,      // OPCODE_LD_ST_VX,
    disassemble_add_i_vx,      // OPCODE_ADD_I_VX,
    disassemble_ld_f_vx,       // OPCODE_LD_F_VX,
    disassemble_ld_b_vx,       // OPCODE_LD_B_VX,
    disassemble_ld_i_vx,       // OPCODE_LD_I_VX,
    disassemble_ld_vx_i,       // OPCODE_LD_VX_I,
    
    // Two-page display for CHIP-8
    disassemble_cls,           // OPCODE_CLS_HIRES

    // S-Chip
    disassemble_invalid, // OPCODE_SCRL_DOWN_N,
    disassemble_invalid, // OPCODE_SCRL_LEFT,
    disassemble_invalid, // OPCODE_SCRL_RIGHT,
    disassemble_invalid, // OPCODE_EXIT,
    disassemble_invalid, // OPCODE_HIDEF_OFF,
    disassemble_invalid, // OPCODE_HIDEF_ON,
    disassemble_invalid, // OPCODE_LD_I_,
    disassemble_invalid, // OPCODE_LD_RPL_VX,
    disassemble_invalid, // OPCODE_LD_VX_RPL,

    // XO-Chip
    disassemble_invalid, // OPCODE_LD_I_VX_VY,
    disassemble_invalid, // OPCODE_LD_VX_VY_I,
    disassemble_invalid, // OPCODE_LD_I_NNNN,
    disassemble_invalid, // OPCODE_DRW_PLN_N,
    disassemble_invalid, // OPCODE_LD_AUDIO_I,
    disassemble_invalid, // OPCODE_SCRL_UP_N,
};

Chip8Error chip8_disassemble(Chip8 *state, FILE *f)
{
    Chip8Opcode opcode;
    Chip8Error error = chip8_decode(state, &opcode, state->PC);

    fprintf(f, "%04x: 0x%04x ", state->PC, opcode.opcode);
    disassemble_arr[opcode.id](&opcode, f);
    fprintf(f, "\n");
    return error;
}