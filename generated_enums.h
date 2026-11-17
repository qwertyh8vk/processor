#include "cmds_and_regs_manual.h"

enum register_nums : uint8_t {
	AX_ASM = 1,
	BX_ASM = 2,
	CX_ASM = 3,
	DX_ASM = 4,
};

enum asm_commands_nums : uint8_t {
	PUSH_ASM = 1,
	POP_ASM  = 2,
	ADD_ASM  = 3,
	SUB_ASM  = 4,
	MUL_ASM  = 5,
	DIV_ASM  = 6,
	SQRT_ASM = 7,
	MOD_ASM  = 8,
	JMP_ASM  = 9,
	JMPA_ASM = 10,
	JMPB_ASM = 11,
	JAE_ASM  = 12,
	JBE_ASM  = 13,
	JMPE_ASM = 14,
	CALL_ASM = 15,
	RET_ASM  = 16,
	IN_ASM   = 17,
	OUT_ASM  = 18,
	HLT_ASM  = 19,
	DUP_ASM  = 20,
	CMP_ASM  = 21,
	OVER_ASM = 22,
	SWAP_ASM = 23,
	DSPL_ASM = 24,
};

enum commands_hash {
	PUSH = 2089443941,
	POP  = 193466804,
	ADD  = 193450094,
	SUB  = 193470255,
	MUL  = 193463731,
	DIV  = 193453544,
	SQRT = 2089547375,
	MOD  = 193463525,
	JMP  = 193460204,
	JMPA = 2089219501,
	JMPB = 2089219502,
	JAE  = 193459797,
	JBE  = 193459830,
	JMPE = 2089219505,
	CALL = 2088954753,
	RET  = 193468656,
	IN   = 5862396,
	OUT  = 193465917,
	HLT  = 193457997,
	DUP  = 193453934,
	CMP  = 193452581,
	OVER = 2089408641,
	SWAP = 2089553344,
	DSPL = 2089010424,
};

enum registers_hash {
	AX = 5862142,
	BX = 5862175,
	CX = 5862208,
	DX = 5862241,
};

typedef struct {
	asm_commands_nums  cmd_asm_code;
	const char*        cmd_string;
	size_t             cmd_string_length;
} enum_and_string_cmd;

typedef struct {
	register_nums  reg_asm_code;
	const char*    reg_string;
	size_t         reg_string_length;
} enum_and_string_reg;

enum_and_string_cmd enum_and_string_cmd_array[N_CMDS] {

	{ PUSH_ASM, "PUSH", 4 },
	{ POP_ASM, "POP", 3 },
	{ ADD_ASM, "ADD", 3 },
	{ SUB_ASM, "SUB", 3 },
	{ MUL_ASM, "MUL", 3 },
	{ DIV_ASM, "DIV", 3 },
	{ SQRT_ASM, "SQRT", 4 },
	{ MOD_ASM, "MOD", 3 },
	{ JMP_ASM, "JMP", 3 },
	{ JMPA_ASM, "JMPA", 4 },
	{ JMPB_ASM, "JMPB", 4 },
	{ JAE_ASM, "JAE", 3 },
	{ JBE_ASM, "JBE", 3 },
	{ JMPE_ASM, "JMPE", 4 },
	{ CALL_ASM, "CALL", 4 },
	{ RET_ASM, "RET", 3 },
	{ IN_ASM, "IN", 2 },
	{ OUT_ASM, "OUT", 3 },
	{ HLT_ASM, "HLT", 3 },
	{ DUP_ASM, "DUP", 3 },
	{ CMP_ASM, "CMP", 3 },
	{ OVER_ASM, "OVER", 4 },
	{ SWAP_ASM, "SWAP", 4 },
	{ DSPL_ASM, "DSPL", 4 },

};

enum_and_string_reg enum_and_string_reg_array[N_REGS] {

	{ AX_ASM, "AX", 2 },
	{ BX_ASM, "BX", 2 },
	{ CX_ASM, "CX", 2 },
	{ DX_ASM, "DX", 2 },

};
