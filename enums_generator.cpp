#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "enum_djb.h"

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
};

const char* const reg_names[] {
    "AX",
    "BX",
    "CX",
    "DX",
};

const int n_regs = (sizeof(reg_names) / sizeof(reg_names[0]));
const int n_cmds = (sizeof(commands) / sizeof(commands[0]));

const int max_len_for_space      = 4;
const int max_len_for_space_regs = 2;

int   enums_header_generator(const char* filename_out);
int   enums_hash_header_generator(const char* filename_out);
void  print_hash_values(FILE* file_out, const char* const ptr_on_array[], const int array_length, const int space_aligment);

int main() {
    const char* output_file = "generated_enums.h";
    
    int enums_header_gen_res      = enums_header_generator(output_file);
    int enums_hash_header_gen_res = enums_hash_header_generator(output_file);

    printf("код ошибки функции: [%d]\n", enums_header_gen_res);
    printf("код ошибки функции hash: [%d]\n", enums_hash_header_gen_res);

    getchar();

    return 0;
}

int enums_header_generator(const char* filename_out) {
    FILE* file_out = fopen(filename_out, "w");

    if (file_out != NULL) {
        printf("файл открылся корректно\n");
        getchar();

        fprintf(file_out, "enum register_nums : uint8_t {\n");// это временное решение, мне оно нужно, пока я фикшу другие штуки

        for (int i = 0; i < n_regs; i++) { fprintf(file_out, "\t%s_ASM = %d,\n", reg_names[i], i + 1); }

        fprintf(file_out, "};\n\n");
        fprintf(file_out, "enum asm_commands_nums : uint8_t {\n");
        // проверка на долбаебушку тут отсутствует, т.к. это ведь мой файл с моими командами, проверка на дибилушку должна быть в ассемблере!
        int how_many_whitespaces = 0;

        for (int array_index = 0; array_index < n_cmds; array_index++) {
            how_many_whitespaces = (int)(max_len_for_space - strlen(commands[array_index]));

            printf("[%d] how many whitespaces\n", how_many_whitespaces);
            getchar();

            fprintf(file_out, "\t%s_ASM%*s = %d,\n", commands[array_index], how_many_whitespaces, "", array_index + 1);
        }

        fprintf(file_out, "};\n\n");
        fclose(file_out);

        return 0;
    }

    else {
        printf("какой-то файл НЕ открылся\n");
        getchar();

        return -1;
    }
}

int enums_hash_header_generator(const char* filename_out) {
    FILE* file_out = fopen(filename_out, "a");

    if (file_out != NULL) {
        fprintf(file_out, "enum commands_hash {\n");
        print_hash_values(file_out, commands, n_cmds, max_len_for_space);
        fprintf(file_out, "};\n\n");
        

        fprintf(file_out, "enum registers_hash {\n");
        print_hash_values(file_out, reg_names, n_regs, max_len_for_space_regs);
        fprintf(file_out, "};\n\n");
        
        fclose(file_out);

        return 0;
    }

    else { return -1; }
}

void print_hash_values(FILE* file_out, const char* const ptr_on_array[], const int array_length, const int space_aligment) {
    for (int array_index = 0; array_index < array_length; array_index++) {
        uint32_t calculated_hash = djb2_calculate_enum_hash(ptr_on_array[array_index]);

        int how_many_whitespaces = (int)(space_aligment - strlen(ptr_on_array[array_index]));

        printf("[%d] how many whitespaces\n", how_many_whitespaces);
        getchar();

        fprintf(file_out, "\t%s%*s = %u,\n", ptr_on_array[array_index], how_many_whitespaces, "", calculated_hash);
    }
}