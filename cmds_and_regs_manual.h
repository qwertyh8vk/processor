#ifndef COMMANDS_H
#define COMMANDS_H

    const char* const commands[] {
        "PUSH",
        "POP",
        "ADD",
        "SUB",
        "MUL",
        "DIV",
        "SQRT",
        "MOD",
        "JMP",
        "JMPA",
        "JMPB",
        "JAE",
        "JBE",
        "JMPE",
        "CALL",
        "RET",
        "IN",
        "OUT",
        "HLT",
        "DUP",
        "CMP",
        "OVER",
        "SWAP",
        "DSPL",
    };

    const char* const reg_names[] {
        "AX",
        "BX",
        "CX",
        "DX",
    };

    enum regs_cmds_amount {
        N_REGS = (sizeof(reg_names) / sizeof(reg_names[0])),
        N_CMDS = (sizeof(commands)  / sizeof(commands[0])),
    };

#endif