#include "enums_generator.h"
#include "conditional_printf.h"

int main() {
    const char* output_file = "generated_enums.h";
    
    int enums_header_gen_res                          = enums_header_generator(output_file);
    int enums_hash_header_gen_res                     = enums_hash_header_generator(output_file);
    int struct_cmds_gen_status = struct_cmds_gen(output_file);
    int struct_regs_gen_status  = struct_regs_gen(output_file); 

    PRINTF("Код ошибки функции header_generator: [%d]\n",                       enums_header_gen_res);
    PRINTF("Код ошибки функции hash_header_generator: [%d]\n",                  enums_hash_header_gen_res);

    PRINTF("Код ошибки функции struct_cmds_gen: [%d]\n", struct_cmds_gen_status);
    PRINTF("Код ошибки функции struct_regs_gen: [%d]\n",  struct_regs_gen_status);

    GETCHAR();

    return 0;
}

int enums_header_generator(const char* filename_out) {
    FILE* file_out = fopen(filename_out, "w");

    if (file_out != NULL) {
        PRINTF("файл открылся корректно\n");
        GETCHAR();
        fprintf(file_out, "#include \"cmds_and_regs_manual.h\"\n\n");
        fprintf(file_out, "enum register_nums : uint8_t {\n");

        for (int i = 0; i < N_REGS; i++) { fprintf(file_out, "\t%s_ASM = %d,\n", reg_names[i], i + 1); }

        fprintf(file_out, "};\n\n");
        fprintf(file_out, "enum asm_commands_nums : uint8_t {\n");
        // проверка на долбаебушку тут отсутствует, т.к. это ведь мой файл с моими командами, проверка на дибилушку должна быть в ассемблере!
        int how_many_whitespaces = 0;

        for (int array_index = 0; array_index < N_CMDS; array_index++) {
            how_many_whitespaces = (int)(max_len_for_space - strlen(commands[array_index]));

            PRINTF("[%d] how many whitespaces\n", how_many_whitespaces);
            GETCHAR();

            fprintf(file_out, "\t%s_ASM%*s = %d,\n", commands[array_index], how_many_whitespaces, "", array_index + 1);
        }

        fprintf(file_out, "};\n\n");
        fclose(file_out);

        return 0;
    }

    else {
        PRINTF("какой-то файл НЕ открылся\n");
        GETCHAR();

        return -1;
    }
}

int enums_hash_header_generator(const char* filename_out) {
    FILE* file_out = fopen(filename_out, "a");

    if (file_out != NULL) {
        fprintf(file_out, "enum commands_hash {\n");
        print_hash_values(file_out, commands, N_CMDS, max_len_for_space);
        fprintf(file_out, "};\n\n");
        

        fprintf(file_out, "enum registers_hash {\n");
        print_hash_values(file_out, reg_names, N_REGS, max_len_for_space_regs);
        fprintf(file_out, "};\n\n");

        // FIXME - перенести
        fprintf(file_out, "typedef struct {\n\tasm_commands_nums  cmd_asm_code;\n\tconst char*        cmd_string;\n\tsize_t             cmd_string_length;\n} enum_and_string_cmd;\n\n");
        fprintf(file_out, "typedef struct {\n\tregister_nums  reg_asm_code;\n\tconst char*    reg_string;\n\tsize_t         reg_string_length;\n} enum_and_string_reg;\n\n");

        fclose(file_out);

        return 0;
    }

    else { return -1; }
}

int struct_cmds_gen(const char* filename_out) {
    FILE* file_out = fopen(filename_out, "a");

    if (file_out != NULL) {
        fprintf(file_out, "enum_and_string_cmd enum_and_string_cmd_array[N_CMDS] {\n\n");

        for (int i = 0; i < N_CMDS; i++) {
            fprintf(file_out, "\t{ %s_ASM, \"%s\", %zu },\n", commands[i], commands[i], strlen(commands[i]));
        }

        fprintf(file_out, "\n};\n\n");
        fclose(file_out);

        return 0;
    }

    else { return -1; }
}

int struct_regs_gen(const char* filename_out) {
    FILE* file_out = fopen(filename_out, "a");

    if (file_out != NULL) {
        fprintf(file_out, "enum_and_string_reg enum_and_string_reg_array[N_REGS] {\n\n");

        for (int i = 0; i < N_REGS; i++) {
            fprintf(file_out, "\t{ %s_ASM, \"%s\", %zu },\n", reg_names[i], reg_names[i], strlen(reg_names[i]));
        }

        fprintf(file_out, "\n};\n");
        fclose(file_out);

        return 0;
    }

    else { return -1; }
}

void print_hash_values(FILE* file_out, const char* const ptr_on_array[], const int array_length, const int space_aligment) {
    for (int array_index = 0; array_index < array_length; array_index++) {
        uint32_t calculated_hash = djb2_calculate_enum_hash(ptr_on_array[array_index]);

        int how_many_whitespaces = (int)(space_aligment - strlen(ptr_on_array[array_index]));

        PRINTF("[%d] how many whitespaces\n", how_many_whitespaces);
        GETCHAR();

        fprintf(file_out, "\t%s%*s = %u,\n", ptr_on_array[array_index], how_many_whitespaces, "", calculated_hash);
    }
}