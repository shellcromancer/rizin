// SPDX-FileCopyrightText: 2022 Dhruv Maroo <dhruvmaru007@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-only

#ifndef RZ_SH_COMMON_H
#define RZ_SH_COMMON_H

#include "disassembler.h"

struct sh_param_builder_addr_t {
	ut8 start; ///< start bit of the param (assuming little-endian)
	SHAddrMode mode; ///< addressing mode being used
};

typedef struct sh_param_builder_t {
	// either find the param using param builder or use an already provided param
	union {
		struct sh_param_builder_addr_t addr;
		SHParam param;
	};
	bool is_param; ///< whether a param was directly passed
} SHParamBuilder;

typedef struct sh_op_raw_t {
	const char *str_mnem; ///< string mnemonic
	SHOpMnem mnemonic; ///< enum mnemonic
	ut16 opcode; ///< opcode
	ut16 mask; ///< mask for opcode to mask out param bits
	SHScaling scaling; ///< scaling for the opcode
	SHParamBuilder param_builder[2]; ///< param builders for the params
} SHOpRaw;

// xxxx used to denote param fields in the opcode
#define I f
#define N f
#define D f
#define M f

// to form opcode in nibbles
#define OPCODE_(a, b, c, d) 0x##a##b##c##d
#define OPCODE(a, b, c, d)  OPCODE_(a, b, c, d)

// nibble position
#define NIB0 0
#define NIB1 4
#define NIB2 8
#define NIB3 12

// return a param builder struct
#define ADDR(nib, addrmode) \
	{ \
		{ .addr = { .start = nib, .mode = addrmode } }, .is_param = false \
	}

// return a param
#define PARAM(reg, addrmode) \
	{ { .param = { .param = { \
			       SH_REG_IND_##reg, \
		       }, \
		    .mode = addrmode } }, \
		.is_param = true }

// opcode for "weird" movl
#define MOVL 0x1fff

#define NOPARAM ADDR(NIB0, SH_ADDR_INVALID)

// Opcode lookup list
static const SHOpRaw sh_op_lookup[] = {
	/* fixed-point transfer instructions */
	{ "mov", SH_OP_MOV, OPCODE(e, N, I, I), 0x0fff, SH_SCALING_INVALID, { ADDR(NIB0, SH_IMM_U), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.w", SH_OP_MOV, OPCODE(9, N, D, D), 0x0fff, SH_SCALING_W, { ADDR(NIB0, SH_PC_RELATIVE_DISP), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.l", SH_OP_MOV, OPCODE(d, N, D, D), 0x0fff, SH_SCALING_L, { ADDR(NIB0, SH_PC_RELATIVE_DISP), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov", SH_OP_MOV, OPCODE(6, N, M, 3), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.b", SH_OP_MOV, OPCODE(2, N, M, 0), 0x0ff0, SH_SCALING_B, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT) } },
	{ "mov.w", SH_OP_MOV, OPCODE(2, N, M, 1), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT) } },
	{ "mov.l", SH_OP_MOV, OPCODE(2, N, M, 2), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT) } },
	{ "mov.b", SH_OP_MOV, OPCODE(6, N, M, 0), 0x0ff0, SH_SCALING_B, { ADDR(NIB1, SH_REG_INDIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.w", SH_OP_MOV, OPCODE(6, N, M, 1), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_INDIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.l", SH_OP_MOV, OPCODE(6, N, M, 2), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_INDIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.b", SH_OP_MOV, OPCODE(2, N, M, 4), 0x0ff0, SH_SCALING_B, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "mov.w", SH_OP_MOV, OPCODE(2, N, M, 5), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "mov.l", SH_OP_MOV, OPCODE(2, N, M, 6), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "mov.b", SH_OP_MOV, OPCODE(6, N, M, 4), 0x0ff0, SH_SCALING_B, { ADDR(NIB1, SH_REG_INDIRECT_I), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.w", SH_OP_MOV, OPCODE(6, N, M, 5), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_INDIRECT_I), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.l", SH_OP_MOV, OPCODE(6, N, M, 6), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_INDIRECT_I), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.b", SH_OP_MOV, OPCODE(8, 0, N, D), 0x00ff, SH_SCALING_B, { PARAM(R0, SH_REG_DIRECT), ADDR(NIB0, SH_REG_INDIRECT_DISP) } },
	{ "mov.w", SH_OP_MOV, OPCODE(8, 1, N, D), 0x00ff, SH_SCALING_W, { PARAM(R0, SH_REG_DIRECT), ADDR(NIB0, SH_REG_INDIRECT_DISP) } },
	{ "mov.l", SH_OP_MOV, OPCODE(1, N, M, D), 0x0fff, SH_SCALING_L, { /*dummy values*/ ADDR(NIB0, SH_ADDR_INVALID), ADDR(NIB0, SH_ADDR_INVALID) } },
	// ^ Just this instruction is kinda weird, so needs to be taken care by sh_op_get_param_movl
	{ "mov.b", SH_OP_MOV, OPCODE(8, 4, M, D), 0x00ff, SH_SCALING_B, { ADDR(NIB0, SH_REG_INDIRECT_DISP), PARAM(R0, SH_REG_DIRECT) } },
	{ "mov.w", SH_OP_MOV, OPCODE(8, 5, M, D), 0x00ff, SH_SCALING_W, { ADDR(NIB0, SH_REG_INDIRECT_DISP), PARAM(R0, SH_REG_DIRECT) } },
	{ "mov.l", SH_OP_MOV, OPCODE(5, N, M, D), 0x0fff, SH_SCALING_L, { ADDR(NIB0, SH_REG_INDIRECT_DISP), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.b", SH_OP_MOV, OPCODE(0, N, M, 4), 0x0ff0, SH_SCALING_B, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_INDEXED) } },
	{ "mov.w", SH_OP_MOV, OPCODE(0, N, M, 5), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_INDEXED) } },
	{ "mov.l", SH_OP_MOV, OPCODE(0, N, M, 6), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_INDEXED) } },
	{ "mov.b", SH_OP_MOV, OPCODE(0, N, M, c), 0x0ff0, SH_SCALING_B, { ADDR(NIB1, SH_REG_INDIRECT_INDEXED), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.w", SH_OP_MOV, OPCODE(0, N, M, d), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_INDIRECT_INDEXED), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.l", SH_OP_MOV, OPCODE(0, N, M, e), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_INDIRECT_INDEXED), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mov.b", SH_OP_MOV, OPCODE(c, 0, D, D), 0x00ff, SH_SCALING_B, { PARAM(R0, SH_REG_DIRECT), ADDR(NIB0, SH_GBR_INDIRECT_DISP) } },
	{ "mov.w", SH_OP_MOV, OPCODE(c, 1, D, D), 0x00ff, SH_SCALING_W, { PARAM(R0, SH_REG_DIRECT), ADDR(NIB0, SH_GBR_INDIRECT_DISP) } },
	{ "mov.l", SH_OP_MOV, OPCODE(c, 2, D, D), 0x00ff, SH_SCALING_L, { PARAM(R0, SH_REG_DIRECT), ADDR(NIB0, SH_GBR_INDIRECT_DISP) } },
	{ "mov.b", SH_OP_MOV, OPCODE(c, 4, D, D), 0x00ff, SH_SCALING_B, { ADDR(NIB0, SH_GBR_INDIRECT_DISP), PARAM(R0, SH_REG_DIRECT) } },
	{ "mov.w", SH_OP_MOV, OPCODE(c, 5, D, D), 0x00ff, SH_SCALING_W, { ADDR(NIB0, SH_GBR_INDIRECT_DISP), PARAM(R0, SH_REG_DIRECT) } },
	{ "mov.l", SH_OP_MOV, OPCODE(c, 6, D, D), 0x00ff, SH_SCALING_L, { ADDR(NIB0, SH_GBR_INDIRECT_DISP), PARAM(R0, SH_REG_DIRECT) } },
	{ "mova", SH_OP_MOV, OPCODE(c, 7, D, D), 0x00ff, SH_SCALING_L, { ADDR(NIB0, SH_PC_RELATIVE_DISP), PARAM(R0, SH_REG_DIRECT) } },
	{ "movt", SH_OP_MOVT, OPCODE(0, N, 2, 9), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "swap.b", SH_OP_SWAP, OPCODE(6, N, M, 8), 0x0ff0, SH_SCALING_B, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "swap.w", SH_OP_SWAP, OPCODE(6, N, M, 9), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "xtrct", SH_OP_XTRCT, OPCODE(2, N, M, d), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },

	/* arithmetic operation instructions */
	{ "add", SH_OP_ADD, OPCODE(3, N, M, c), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "add", SH_OP_ADD, OPCODE(7, N, I, I), 0x0fff, SH_SCALING_INVALID, { ADDR(NIB0, SH_IMM_S), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "addc", SH_OP_ADDC, OPCODE(3, N, M, e), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "addv", SH_OP_ADDV, OPCODE(3, N, M, f), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "cmp/eq", SH_OP_CMP_EQ, OPCODE(8, 8, I, I), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_IMM_S), PARAM(R0, SH_REG_DIRECT) } },
	{ "cmp/eq", SH_OP_CMP_EQ, OPCODE(3, N, M, 0), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "cmp/hs", SH_OP_CMP_HS, OPCODE(3, N, M, 2), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "cmp/ge", SH_OP_CMP_GE, OPCODE(3, N, M, 3), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "cmp/hi", SH_OP_CMP_HI, OPCODE(3, N, M, 6), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "cmp/gt", SH_OP_CMP_GT, OPCODE(3, N, M, 7), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "cmp/pz", SH_OP_CMP_PZ, OPCODE(4, N, 1, 1), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "cmp/pl", SH_OP_CMP_PL, OPCODE(4, N, 1, 5), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "cmp/str", SH_OP_CMP_STR, OPCODE(2, N, M, c), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "div1", SH_OP_DIV1, OPCODE(3, N, M, 4), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "div0s", SH_OP_DIV0S, OPCODE(2, N, M, 7), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "div0u", SH_OP_DIV0U, OPCODE(0, 0, 1, 9), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "dmuls.l", SH_OP_DMULS, OPCODE(3, N, M, d), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "dmulu.l", SH_OP_DMULU, OPCODE(3, N, M, 5), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "dt", SH_OP_DT, OPCODE(4, N, 1, 0), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "exts.b", SH_OP_EXTS, OPCODE(6, N, M, e), 0x0ff0, SH_SCALING_B, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "exts.w", SH_OP_EXTS, OPCODE(6, N, M, f), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "extu.b", SH_OP_EXTU, OPCODE(6, N, M, c), 0x0ff0, SH_SCALING_B, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "extu.w", SH_OP_EXTU, OPCODE(6, N, M, d), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mac.l", SH_OP_MAC, OPCODE(0, N, M, f), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_INDIRECT_I), ADDR(NIB2, SH_REG_INDIRECT_I) } },
	{ "mac.w", SH_OP_MAC, OPCODE(4, N, M, f), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_INDIRECT_I), ADDR(NIB2, SH_REG_INDIRECT_I) } },
	{ "mul.l", SH_OP_MUL, OPCODE(0, N, M, 7), 0x0ff0, SH_SCALING_L, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "muls.w", SH_OP_MULS, OPCODE(2, N, M, f), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "mulu.w", SH_OP_MULU, OPCODE(2, N, M, e), 0x0ff0, SH_SCALING_W, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "neg", SH_OP_NEG, OPCODE(6, N, M, b), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "negc", SH_OP_NEGC, OPCODE(6, N, M, a), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "sub", SH_OP_SUB, OPCODE(3, N, M, 8), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "subc", SH_OP_SUBC, OPCODE(3, N, M, a), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "subv", SH_OP_SUBV, OPCODE(3, N, M, b), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },

	/* logic operation instructions */
	{ "and", SH_OP_AND, OPCODE(2, N, M, 9), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "and", SH_OP_AND, OPCODE(c, 9, I, I), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_IMM_U), PARAM(R0, SH_REG_DIRECT) } },
	{ "and.b", SH_OP_AND, OPCODE(c, d, I, I), 0x00ff, SH_SCALING_B, { ADDR(NIB0, SH_IMM_U), PARAM(R0, SH_GBR_INDIRECT_INDEXED) } },
	{ "not", SH_OP_NOT, OPCODE(6, N, M, 7), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "or", SH_OP_OR, OPCODE(2, N, M, b), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "or", SH_OP_OR, OPCODE(c, b, I, I), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_IMM_U), PARAM(R0, SH_REG_DIRECT) } },
	{ "or.b", SH_OP_OR, OPCODE(c, f, I, I), 0x00ff, SH_SCALING_B, { ADDR(NIB0, SH_IMM_U), PARAM(R0, SH_GBR_INDIRECT_INDEXED) } },
	{ "tas.b", SH_OP_TAS, OPCODE(4, N, 1, a), 0x0f00, SH_SCALING_B, { ADDR(NIB2, SH_REG_INDIRECT), NOPARAM } },
	{ "tst", SH_OP_TST, OPCODE(2, N, M, 8), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "tst", SH_OP_TST, OPCODE(c, 8, I, I), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_IMM_U), PARAM(R0, SH_REG_DIRECT) } },
	{ "tst.b", SH_OP_TST, OPCODE(c, c, I, I), 0x00ff, SH_SCALING_B, { ADDR(NIB0, SH_IMM_U), PARAM(R0, SH_GBR_INDIRECT_INDEXED) } },
	{ "xor", SH_OP_XOR, OPCODE(2, N, M, a), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "xor", SH_OP_XOR, OPCODE(c, a, I, I), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_IMM_U), PARAM(R0, SH_REG_DIRECT) } },
	{ "xor.b", SH_OP_XOR, OPCODE(c, e, I, I), 0x00ff, SH_SCALING_B, { ADDR(NIB0, SH_IMM_U), PARAM(R0, SH_GBR_INDIRECT_INDEXED) } },

	/* shift instructions */
	{ "rotl", SH_OP_ROTL, OPCODE(4, N, 0, 4), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "rotr", SH_OP_ROTR, OPCODE(4, N, 0, 5), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "rotcl", SH_OP_ROTCL, OPCODE(4, N, 2, 4), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "rotcr", SH_OP_ROTCR, OPCODE(4, N, 2, 5), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shad", SH_OP_SHAD, OPCODE(4, N, M, c), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "shal", SH_OP_SHAL, OPCODE(4, N, 4, 0), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shar", SH_OP_SHAR, OPCODE(4, N, 4, 1), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shld", SH_OP_SHLD, OPCODE(4, N, M, c), 0x0ff0, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "shll", SH_OP_SHLL, OPCODE(4, N, 0, 0), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shlr", SH_OP_SHLR, OPCODE(4, N, 0, 1), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shll2", SH_OP_SHLL2, OPCODE(4, N, 0, 8), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shlr2", SH_OP_SHLR2, OPCODE(4, N, 0, 9), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shll8", SH_OP_SHLL8, OPCODE(4, N, 1, 8), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shlr8", SH_OP_SHLR8, OPCODE(4, N, 1, 9), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shll16", SH_OP_SHLL16, OPCODE(4, N, 2, 8), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },
	{ "shlr16", SH_OP_SHLR16, OPCODE(4, N, 2, 9), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), NOPARAM } },

	/* branch instructions */
	{ "bf", SH_OP_BF, OPCODE(8, b, D, D), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_PC_RELATIVE8), NOPARAM } },
	{ "bf/s", SH_OP_BFS, OPCODE(8, f, D, D), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_PC_RELATIVE8), NOPARAM } },
	{ "bt", SH_OP_BT, OPCODE(8, 9, D, D), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_PC_RELATIVE8), NOPARAM } },
	{ "bt/s", SH_OP_BTS, OPCODE(8, d, D, D), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_PC_RELATIVE8), NOPARAM } },
	{ "bra", SH_OP_BRA, OPCODE(a, D, D, D), 0x0fff, SH_SCALING_INVALID, { ADDR(NIB0, SH_PC_RELATIVE12), NOPARAM } },
	{ "braf", SH_OP_BRAF, OPCODE(0, N, 2, 3), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_PC_RELATIVE_REG), NOPARAM } },
	{ "bsr", SH_OP_BSR, OPCODE(b, D, D, D), 0x0fff, SH_SCALING_INVALID, { ADDR(NIB0, SH_PC_RELATIVE12), NOPARAM } },
	{ "bsrf", SH_OP_BSRF, OPCODE(0, N, 0, 3), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_PC_RELATIVE_REG), NOPARAM } },
	{ "jmp", SH_OP_JMP, OPCODE(4, N, 2, b), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_INDIRECT), NOPARAM } },
	{ "jsr", SH_OP_JSR, OPCODE(4, N, 0, b), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_INDIRECT), NOPARAM } },
	// ^ jmp and jsr disassembly should ideally print @Rn (indirect access), but that doesn't make sense from the analysis POV
	// so for the sake of simplicity, only Rn (direct access) will be printed
	{ "rts", SH_OP_JSR, OPCODE(0, 0, 0, b), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },

	/* system control instructions */
	{ "clrmac", SH_OP_CLRMAC, OPCODE(0, 0, 2, 8), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "clrs", SH_OP_CLRS, OPCODE(0, 0, 4, 8), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "clrt", SH_OP_CLRT, OPCODE(0, 0, 0, 8), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "ldc", SH_OP_LDC, OPCODE(4, M, 0, e), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), PARAM(SR, SH_REG_DIRECT) } },
	{ "ldc", SH_OP_LDC, OPCODE(4, M, 1, e), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), PARAM(GBR, SH_REG_DIRECT) } },
	{ "ldc", SH_OP_LDC, OPCODE(4, M, 2, e), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), PARAM(VBR, SH_REG_DIRECT) } },
	{ "ldc", SH_OP_LDC, OPCODE(4, M, 3, e), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), PARAM(SSR, SH_REG_DIRECT) } },
	{ "ldc", SH_OP_LDC, OPCODE(4, M, 4, e), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), PARAM(SPC, SH_REG_DIRECT) } },
	{ "ldc", SH_OP_LDC, OPCODE(4, M, f, a), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), PARAM(DBR, SH_REG_DIRECT) } },
	{ "ldc", SH_OP_UNIMPL, OPCODE(4, M, N, e), 0x0f70, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), ADDR(NIB1, SH_REG_DIRECT) } },
	{ "ldc.l", SH_OP_LDC, OPCODE(4, M, 0, 7), 0x0f00, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), PARAM(SR, SH_REG_DIRECT) } },
	{ "ldc.l", SH_OP_LDC, OPCODE(4, M, 1, 7), 0x0f00, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), PARAM(GBR, SH_REG_DIRECT) } },
	{ "ldc.l", SH_OP_LDC, OPCODE(4, M, 2, 7), 0x0f00, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), PARAM(VBR, SH_REG_DIRECT) } },
	{ "ldc.l", SH_OP_LDC, OPCODE(4, M, 3, 7), 0x0f00, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), PARAM(SSR, SH_REG_DIRECT) } },
	{ "ldc.l", SH_OP_LDC, OPCODE(4, M, 4, 7), 0x0f00, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), PARAM(SPC, SH_REG_DIRECT) } },
	{ "ldc.l", SH_OP_LDC, OPCODE(4, M, f, 6), 0x0f00, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), PARAM(DBR, SH_REG_DIRECT) } },
	{ "ldc.l", SH_OP_UNIMPL, OPCODE(4, M, N, 7), 0x0f70, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), ADDR(NIB1, SH_REG_DIRECT) } },
	{ "lds", SH_OP_LDS, OPCODE(4, M, 0, a), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), PARAM(MACH, SH_REG_DIRECT) } },
	{ "lds", SH_OP_LDS, OPCODE(4, M, 1, a), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), PARAM(MACL, SH_REG_DIRECT) } },
	{ "lds", SH_OP_LDS, OPCODE(4, M, 2, a), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_DIRECT), PARAM(PR, SH_REG_DIRECT) } },
	{ "lds.l", SH_OP_LDS, OPCODE(4, M, 0, 6), 0x0f00, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), PARAM(MACH, SH_REG_DIRECT) } },
	{ "lds.l", SH_OP_LDS, OPCODE(4, M, 1, 6), 0x0f00, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), PARAM(MACL, SH_REG_DIRECT) } },
	{ "lds.l", SH_OP_LDS, OPCODE(4, M, 2, 6), 0x0f00, SH_SCALING_L, { ADDR(NIB2, SH_REG_INDIRECT_I), PARAM(PR, SH_REG_DIRECT) } },
	{ "ldtlb", SH_OP_UNIMPL, OPCODE(0, 0, 3, 8), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "movca.l", SH_OP_MOVCA, OPCODE(0, N, c, 3), 0x0f00, SH_SCALING_L, { PARAM(R0, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT) } },
	{ "nop", SH_OP_NOP, OPCODE(0, 0, 0, 9), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "ocbi", SH_OP_UNIMPL, OPCODE(0, N, 9, 3), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_INDIRECT), NOPARAM } },
	{ "ocbp", SH_OP_UNIMPL, OPCODE(0, N, a, 3), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_INDIRECT), NOPARAM } },
	{ "ocbwb", SH_OP_UNIMPL, OPCODE(0, N, b, 3), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_INDIRECT), NOPARAM } },
	{ "pref", SH_OP_UNIMPL, OPCODE(0, N, 8, 3), 0x0f00, SH_SCALING_INVALID, { ADDR(NIB2, SH_REG_INDIRECT), NOPARAM } },
	{ "rte", SH_OP_RTE, OPCODE(0, 0, 2, b), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "sets", SH_OP_SETS, OPCODE(0, 0, 5, 8), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "sett", SH_OP_SETT, OPCODE(0, 0, 1, 8), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "sleep", SH_OP_SLEEP, OPCODE(0, 0, 1, b), 0x0000, SH_SCALING_INVALID, { NOPARAM, NOPARAM } },
	{ "stc", SH_OP_STC, OPCODE(0, M, 0, 2), 0x0f00, SH_SCALING_INVALID, { PARAM(SR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "stc", SH_OP_STC, OPCODE(0, M, 1, 2), 0x0f00, SH_SCALING_INVALID, { PARAM(GBR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "stc", SH_OP_STC, OPCODE(0, M, 2, 2), 0x0f00, SH_SCALING_INVALID, { PARAM(VBR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "stc", SH_OP_STC, OPCODE(0, M, 3, 2), 0x0f00, SH_SCALING_INVALID, { PARAM(SSR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "stc", SH_OP_STC, OPCODE(0, M, 4, 2), 0x0f00, SH_SCALING_INVALID, { PARAM(SPC, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "stc", SH_OP_STC, OPCODE(0, M, 3, a), 0x0f00, SH_SCALING_INVALID, { PARAM(SGR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "stc", SH_OP_STC, OPCODE(0, M, f, a), 0x0f00, SH_SCALING_INVALID, { PARAM(DBR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "stc", SH_OP_UNIMPL, OPCODE(0, N, M, 2), 0x0f70, SH_SCALING_INVALID, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "stc.l", SH_OP_STC, OPCODE(4, M, 0, 3), 0x0f00, SH_SCALING_L, { PARAM(SR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "stc.l", SH_OP_STC, OPCODE(4, M, 1, 3), 0x0f00, SH_SCALING_L, { PARAM(GBR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "stc.l", SH_OP_STC, OPCODE(4, M, 2, 3), 0x0f00, SH_SCALING_L, { PARAM(VBR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "stc.l", SH_OP_STC, OPCODE(4, M, 3, 3), 0x0f00, SH_SCALING_L, { PARAM(SSR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "stc.l", SH_OP_STC, OPCODE(4, M, 4, 3), 0x0f00, SH_SCALING_L, { PARAM(SPC, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "stc.l", SH_OP_STC, OPCODE(4, M, 3, 2), 0x0f00, SH_SCALING_L, { PARAM(SGR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "stc.l", SH_OP_STC, OPCODE(4, M, f, 2), 0x0f00, SH_SCALING_L, { PARAM(DBR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "stc.l", SH_OP_UNIMPL, OPCODE(4, N, M, 3), 0x0f70, SH_SCALING_L, { ADDR(NIB1, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "sts", SH_OP_STS, OPCODE(0, N, 0, a), 0x0f00, SH_SCALING_INVALID, { PARAM(MACH, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "sts", SH_OP_STS, OPCODE(0, N, 1, a), 0x0f00, SH_SCALING_INVALID, { PARAM(MACL, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "sts", SH_OP_STS, OPCODE(0, N, 2, a), 0x0f00, SH_SCALING_INVALID, { PARAM(PR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_DIRECT) } },
	{ "sts.l", SH_OP_STS, OPCODE(4, N, 0, 2), 0x0f00, SH_SCALING_L, { PARAM(MACH, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "sts.l", SH_OP_STS, OPCODE(4, N, 1, 2), 0x0f00, SH_SCALING_L, { PARAM(MACL, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "sts.l", SH_OP_STS, OPCODE(4, N, 2, 2), 0x0f00, SH_SCALING_L, { PARAM(PR, SH_REG_DIRECT), ADDR(NIB2, SH_REG_INDIRECT_D) } },
	{ "trapa", SH_OP_UNIMPL, OPCODE(c, 3, I, I), 0x00ff, SH_SCALING_INVALID, { ADDR(NIB0, SH_IMM_U), NOPARAM } }

	/* floating-point single-precision instructions */
	// TODO
};

#undef NOPARAM
#undef PARAM
#undef ADDR
#undef OPCODE
#undef OPCODE_
#undef M
#undef D
#undef N
#undef I

static const ut32 OPCODE_NUM = sizeof(sh_op_lookup) / sizeof(SHOpRaw);

#endif // RZ_SH_COMMON_H
