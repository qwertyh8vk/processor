#include "assembler_fscanf.h"
#include "enum_djb.h"

int             main() {

    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
    // СЛОВЕСНАЯ МЕТКА - ЭТО СТРУКТУРА, КОТОРАЯ СОДЕРЖИТ ИМЯ МЕТКИ И ИНДЕКС, НА КОТОРЫЙ ОНА УКАЗЫВАЕТ. //
    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //

    const char*    to_be_assemblered                 = "assembler.txt";
    const char*    assemblered_file                  = "assembler_res.txt";
    const char*    assemblered_file_after_first_asm  = "assembler_res_after_first.txt";

    struct stat    to_be_assemblered_struct;

    char*          file_buffer       = initialize_file_buffer(to_be_assemblered, &to_be_assemblered_struct);
    long long      bytecode_capacity = to_be_assemblered_struct.st_size + 1;
    uint8_t*       bytecode          = (uint8_t *)calloc(bytecode_capacity, sizeof(char));
    
    bytecode[to_be_assemblered_struct.st_size] = '\0';

    if (file_buffer != NULL) {
        potential_error error_after_load = load_file_buffer(to_be_assemblered, file_buffer, to_be_assemblered_struct.st_size);
        if (error_after_load != OK) { return error_after_load; }
        debugger_like_printf(file_buffer, &to_be_assemblered_struct);
    }

    potential_error obrabotka_faila_status = obrabotka_faila(file_buffer, bytecode, assemblered_file_after_first_asm, bytecode_capacity);
   
    for (off_t i = 0; i < bytecode_capacity; i++) {
        printf("<%u> ", bytecode[i]);
    }
    printf("\n");

    return obrabotka_faila_status;
}

potential_error obrabotka_faila(char* file_buffer,  uint8_t* bytecode, const char* filename_out_after_first_asm, long long bytecode_capacity) {
    long long labels_array[MAX_LABELS_AMOUNT] = {0};

    for (int i = 0; i < MAX_LABELS_AMOUNT; i++) { labels_array[i] = -1; }

    size_t          commands_and_arguments_counted = 0;
    potential_error assemble_file_stage_1 = assemble_file_second(file_buffer, bytecode, labels_array, FIRST, &commands_and_arguments_counted, bytecode_capacity);

    PRINTF("Ошибка после первого ассембла: [%d]\n", assemble_file_stage_1);
    GETCHAR();
    PRINTF("Количество прочитанных символов: [%zu]\n", commands_and_arguments_counted);
    GETCHAR(); 

    potential_error assemble_file_stage_2 = assemble_file_second(file_buffer, bytecode, labels_array, SECOND, &commands_and_arguments_counted, bytecode_capacity);

    PRINTF("Ошибка после второго ассембла: [%d]\n", assemble_file_stage_2);
    GETCHAR();

    return OK;
}

char*           initialize_file_buffer(const char* filename, struct stat* file_buffer_struct) {
    assert(filename);

    int stat_status = stat(filename, file_buffer_struct);

    if (stat_status == 0) {
        off_t file_buffer_size = file_buffer_struct->st_size;
        char* file_buffer = (char *)calloc(file_buffer_size + 1, sizeof(char));

        if (file_buffer != NULL) {
            PRINTF("успешно закаллокали\n");
            GETCHAR();

            return file_buffer;
        }
        
        else { PRINTF("проблема с каллоком\n"); return NULL; }
    }

    else { PRINTF("проблемы со статом\n"); return NULL; }

}

potential_error load_file_buffer(const char* filename, char* file_buffer, off_t struct_size) {
    int file_descr = open(filename, O_RDONLY);

    if (file_descr != -1) {
        ssize_t bytes_read = read(file_descr, file_buffer, struct_size);
        
        if (bytes_read == struct_size) {
            PRINTF("всё окей, размеры совпадают\n");
            GETCHAR();
            
            file_buffer[struct_size] = '\0';
            bytes_read++;

            return OK;
        }

        else {
            PRINTF("что-то пошло не так\n");
            GETCHAR();

            return COMPARISON_FAILED;
        }
    }

    else {
        PRINTF("проблема с open\n");

        return FILE_NOT_OPENNED;
    }
}

potential_error assemble_file_second(char*        buffer_in,    uint8_t* bytecode, long long* labels_array, 
                                     progon_index progon_index, size_t*  commands_and_arguments_counted, 
                                     long long    bytecode_capacity) 
{
    assert(buffer_in);
    assert(bytecode);

    long long  instruction_pointer = 0;
    char       single_command_line[MAX_LINE + 1] = {};
    int        debug_index                       = 0;

    while (sscanf(buffer_in, "%256[^\n]%n", single_command_line, &debug_index) == 1) {

        // прочитал строку в массив single_command_line

        /*
            1) проверил строку на корректность по длине - сделано
            2) если корректна - вытащил команду - сделано
            3) если такая команда есть - обработал операнды индивидуально по схеме
            4) если операнды корректные - записал строку и прибавил счётчик команд через побитовую маску
        */

        potential_error check_length_status = validate_string_length(buffer_in, debug_index);

        if (check_length_status == OK) {
            PRINTF("ДЕБАГ ИНДЕКС: <%d>\n", debug_index);
            GETCHAR();
            debug_index += 1; // перешагнули через \n
            buffer_in   += debug_index;

            PRINTF("assemble_file_second ||\nbuffer_in ||\nГДЕ Я:\n<%s>, <%c>\n", buffer_in, *buffer_in);
            GETCHAR();
            // 1 шаг выполнили

            int      command_length = 0;
            uint32_t command_hash   = pull_the_fucking_command(single_command_line, &command_length);
            PRINTF("<%d> command_length\n", command_length);
            // 2 шаг выполнили . вытащили команду

            // индекс сдвинутый на длину команды (ничего более)
            char* command_line_ptr                   = single_command_line + command_length;
            PRINTF("assemble_file_second: <%s> single_command_line\n", single_command_line);
            GETCHAR(); 
            PRINTF("assemble_file_second: <%s> command_line_ptr\n", command_line_ptr);
            GETCHAR();
            potential_error process_command_status   = process_command_byhash(command_hash,          single_command_line, 
                                                                              command_line_ptr,      labels_array, 
                                                                              &instruction_pointer,  bytecode, 
                                                                              bytecode_capacity);
             
            PRINTF("process_command_status: <%d>\n", process_command_status);
            GETCHAR();
            if (process_command_status == OK) {
                PRINTF("команда успешно обработалась\n");
                GETCHAR();
            }

            else { return process_command_status; }
        }

        clear_command_array(single_command_line);
        
    }

    if (progon_index == FIRST) { *commands_and_arguments_counted = (instruction_pointer + 1); }

    return OK;
}

potential_error process_command_byhash(uint32_t      command_hash,         char*           single_command_line, 
                                        char*        command_line_ptr,     long long*      labels_array,
                                        long long*   instruction_pointer,  uint8_t*        bytecode, 
                                        long long    bytecode_capacity) {
    switch (command_hash) {

        MACROS_PROCESS_PUSH_POP_BY_HASH(PUSH);
        MACROS_PROCESS_PUSH_POP_BY_HASH(POP);

        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(ADD);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(SUB);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(MUL);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(DIV);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(SQRT);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(MOD);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(RET);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(IN);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(OUT);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(HLT);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(DUP);
        MACROS_PROCESS_ZERO_OPERAND_BY_HASH(CMP);

        MACROS_PROCESS_LABEL_COMMANDS_BY_HASH(JMP);
        MACROS_PROCESS_LABEL_COMMANDS_BY_HASH(JMPA);
        MACROS_PROCESS_LABEL_COMMANDS_BY_HASH(JMPB);
        MACROS_PROCESS_LABEL_COMMANDS_BY_HASH(JAE);
        MACROS_PROCESS_LABEL_COMMANDS_BY_HASH(JBE);
        MACROS_PROCESS_LABEL_COMMANDS_BY_HASH(JMPE);
        MACROS_PROCESS_LABEL_COMMANDS_BY_HASH(CALL);
        
        MACROS_DEFAULT_SITUATION();
        
    }

    return OK;
}


potential_error write_operand(asm_commands_nums func_asm_code,
                              uint8_t                   flags,
                              register_nums               reg,
                              STACK_DATA_TYPE          chislo,
                              uint8_t*               bytecode,
                              long long*  instruction_pointer,
                              long long     bytecode_capacity)
{
    uint8_t func_asm_code_uint = (uint8_t)func_asm_code;

    func_asm_code_uint = func_asm_code_uint | flags;

    if ((*instruction_pointer < bytecode_capacity) && (bytecode_capacity - *instruction_pointer > sizeof(uint8_t))) {
        memcpy(bytecode + *instruction_pointer, &func_asm_code_uint, sizeof(uint8_t));
        *instruction_pointer += sizeof(uint8_t);
    }

    else { PRINTF("попытка записать func_asm_code_uint: недостаточно места\n"); GETCHAR(); return BYTECODE_OVERFLOW; }

    if (flags & REGISTER_BIT) {
        PRINTF("Мы попали в регистер бит\n");
        GETCHAR();

        uint8_t reg_enum_uint  = (uint8_t)reg;
        
        if ((*instruction_pointer < bytecode_capacity) && (bytecode_capacity - *instruction_pointer > sizeof(uint8_t))) {
            memcpy(bytecode + *instruction_pointer, &reg_enum_uint, sizeof(uint8_t));
            *instruction_pointer += sizeof(uint8_t);
        }

        else { PRINTF("попытка записать reg_enum_uint: недостаточно места\n"); GETCHAR(); return BYTECODE_OVERFLOW; }
    }

    if (flags & CONST_BIT) {
        PRINTF("Мы попали в конст бит\n");
        GETCHAR();

        if ((*instruction_pointer < bytecode_capacity) && (bytecode_capacity - *instruction_pointer > sizeof(STACK_DATA_TYPE))) {
            memcpy(bytecode + *instruction_pointer, &chislo, sizeof(STACK_DATA_TYPE));
            *instruction_pointer += sizeof(STACK_DATA_TYPE);
        }
        
        else { PRINTF("попытка записать chislo: недостаточно места\n"); GETCHAR(); return BYTECODE_OVERFLOW; }
    }

    return OK;
}

potential_error parse_operand_push_pop(char*               command_line_ptr, 
                                       operand_data*  ptr_on_operand_struct, 
                                       int*                     debug_index,
                                       asm_commands_nums      func_asm_code) 
{
    char reg[MAX_COMMAND_LEN] = {};
    int  register_flag        = 0;
    char char_for_pop         = 0;

    if (sscanf(command_line_ptr, " [%15[^ ] + %lf]%n", reg, &(ptr_on_operand_struct->number), debug_index) == 2 ||
        sscanf(command_line_ptr, " [%lf + %15[^]]]%n", &(ptr_on_operand_struct->number), reg, debug_index) == 2 ) 
    {
        PRINTF("считавшиеся число и регистр: <%lf> и <%s>\n", ptr_on_operand_struct->number, reg);
        GETCHAR();
        ptr_on_operand_struct->flags = MEMORY_BIT | CONST_BIT | REGISTER_BIT;
        register_flag++;
    } 

    else if (sscanf(command_line_ptr, " [%lf]%n", &(ptr_on_operand_struct->number), debug_index) == 1) {
        PRINTF("считавшееся число: <%lf>\n", ptr_on_operand_struct->number);
        GETCHAR();
        ptr_on_operand_struct->flags = MEMORY_BIT | CONST_BIT;
    }

    else if (sscanf(command_line_ptr, " [%15[^]]]%n", reg, debug_index) == 1) {
        PRINTF("считавшийся регистр (1): <%s>\n", reg);
        GETCHAR();
        ptr_on_operand_struct->flags = MEMORY_BIT | REGISTER_BIT;
        register_flag++;
    } 

    else if (sscanf(command_line_ptr, "%lf + %15s%n", &(ptr_on_operand_struct->number), reg, debug_index) == 2 || 
             sscanf(command_line_ptr, "%15s + %lf%n", reg, &(ptr_on_operand_struct->number), debug_index) == 2) 
    {
        if (func_asm_code == PUSH_ASM) {
            PRINTF("считавшееся число и регистр: <%lf> и <%s>\n", ptr_on_operand_struct->number, reg);
            GETCHAR();
            ptr_on_operand_struct->flags = CONST_BIT | REGISTER_BIT;
            register_flag++;
        }

        else { PRINTF("недопустимый операнд (регистр + константа для попа)\n"); return INCORRECT_COMMAND_LINE; }
    } 

    else if (sscanf(command_line_ptr, "%15s%n", reg, debug_index) == 1) {
        PRINTF("считавшийся регистр (2): %s\n", reg);
        GETCHAR();
        ptr_on_operand_struct->flags = REGISTER_BIT;
        register_flag++;
    } 

    else if (sscanf(command_line_ptr, "%lf%n", &(ptr_on_operand_struct->number), debug_index) == 1) {
        if (func_asm_code == PUSH_ASM) {
            PRINTF("считавшееся число: <%lf>\n", ptr_on_operand_struct->number);
            GETCHAR();
            ptr_on_operand_struct->flags = CONST_BIT;
            register_flag++;
        }

        else { PRINTF("недопустимый операнд (константа для попа)\n"); return INCORRECT_COMMAND_LINE; }
    } 
    
    else if ((sscanf(command_line_ptr, "%c", &char_for_pop) == 1) && char_for_pop == '\n') {
        if (func_asm_code == POP_ASM) { ; }

        else { PRINTF("недопустимый операнд (пусто для пуша)\n"); return INCORRECT_COMMAND_LINE; }
    }

    else {
        PRINTF("нераспознанный формат ввода после push/pop'a\n");
        GETCHAR();
        
        return INCORRECT_COMMAND_LINE;
    }

    if (register_flag != 0) {
        register_nums reg_enum = determine_register(reg);
        if (reg_enum < 0) { PRINTF("некорректный (нераспознанный) регистр.\n"); return INCORRECT_REG; }
        ptr_on_operand_struct->reg = reg_enum;
    }

    return OK;
}

potential_error validate_push_pop_command(char*              command_line_ptr, 
                                          asm_commands_nums  func_asm_code, 
                                          uint8_t*           bytecode,
                                          long long*         instruction_pointer,
                                          long long          bytecode_capacity) 
{
    PRINTF("validate_push_pop_command (1)\nна каком мы сейчас символе: [%c], [%s]\n", *command_line_ptr, command_line_ptr);
    GETCHAR();

    operand_data operand_data_struct = {};
    int debug_index = 0;

    potential_error parse_operand_status = parse_operand_push_pop(command_line_ptr, &operand_data_struct, &debug_index, func_asm_code);
    if (parse_operand_status == OK) {
        char* pointer_on_str_after_args = command_line_ptr + debug_index;

        PRINTF("<%s>\n", command_line_ptr);
          
        PRINTF("ДЕБАГ ИНДЕКС ПЕРЕД ВЕЛИДЕЙТОМ: <%d>\n", debug_index);
        PRINTF("validate_push_pop_command (2)\nна каком мы сейчас символе: <%c>, <%s>\n", *pointer_on_str_after_args, pointer_on_str_after_args);
        GETCHAR();

        potential_error check_string_after_arg_status = check_string_after_arg(pointer_on_str_after_args);

        if (check_string_after_arg_status == OK) {
            PRINTF("вот вот сделаю write operand\n");
            GETCHAR();

            potential_error write_operand_status = write_operand(func_asm_code,           operand_data_struct.flags, 
                                                                 operand_data_struct.reg, operand_data_struct.number, 
                                                                 bytecode,                instruction_pointer, 
                                                                 bytecode_capacity);

            PRINTF("<%d> write operand status\n", write_operand_status);
            return write_operand_status;
        }

        else { return check_string_after_arg_status; }
    }

    else { return parse_operand_status; }
}

register_nums   determine_register(char* reg) {
    PRINTF("[%s], наш регистр", reg);
    GETCHAR();

    uint32_t register_hash = djb2_calculate_enum_hash(reg);

    switch  (register_hash)
    {
        case AX:
            return AX_ASM;

        case BX:
            return BX_ASM;

        case CX:
            return CX_ASM;
        
        case DX:
            return DX_ASM;

        default:
            PRINTF("Регистр не распознан\n");
            GETCHAR();

            return (register_nums)-1;
    }
}

potential_error validate_zero_operands_commands(char*              command_line_ptr, 
                                                asm_commands_nums  func_asm_code, 
                                                uint8_t*           bytecode,
                                                long long*         instruction_pointer,
                                                long long          bytecode_capacity) 
{
    PRINTF("validate_zero_operands_commands (1)\nна каком мы сейчас символе: [%c], [%s]\n", *command_line_ptr, command_line_ptr);
    GETCHAR();

    while (*command_line_ptr == ' ') {
        command_line_ptr++;
    }

    if (*command_line_ptr == '\0') {
        PRINTF("проскипав пробелы мы дошли до слеш0, всё в порядке\n");
        GETCHAR();

        operand_data operand_data_struct = {};

        potential_error write_operand_status = write_operand(func_asm_code,                 operand_data_struct.flags, 
                                                             operand_data_struct.reg,       operand_data_struct.number, 
                                                             bytecode, instruction_pointer, bytecode_capacity);

        return write_operand_status;    
    }
    
    else {
        PRINTF("что-то есть после команды: [%c], [%s]\n", *command_line_ptr, command_line_ptr);
        GETCHAR();

        return INCORRECT_COMMAND_LINE;
    }
}

potential_error check_string_after_arg(char* command_line_ptr) {
    PRINTF("check_string_after_arg (1)\nна каком мы сейчас символе: [%c], [%s]\n", *command_line_ptr, command_line_ptr);
    GETCHAR();

    while (*command_line_ptr == ' ') { command_line_ptr++; }

    if (*command_line_ptr == '\0')   { return OK; }
    else                             { return INCORRECT_COMMAND_LINE; }
}

potential_error validate_commands_only_labels(char*             command_line_ptr, 
                                              long long*        labels_array, 
                                              asm_commands_nums func_asm_code,
                                              uint8_t*          bytecode,
                                              long long*        instruction_pointer,
                                              long long         bytecode_capacity) 
{
    PRINTF("validate_commands_only_labels (1)\nна каком мы сейчас символе: [%c], [%s]\n", *command_line_ptr, command_line_ptr);
    GETCHAR();
    
    while (*command_line_ptr == ' ') {
        command_line_ptr++;
    }

    if (*command_line_ptr == ':') {
        PRINTF("проскипав пробелы мы дошли до двоеточия, всё в порядке\n");
        GETCHAR();

        command_line_ptr++;
        char*     end_pointer = NULL;
        long long label_index = strtoll(command_line_ptr, &end_pointer, 10);

        if (end_pointer == command_line_ptr) {
            PRINTF("число не найдено, end_ptr = command_line_ptr\n");
            GETCHAR();

            return INCORRECT_COMMAND_LINE;
        }

        PRINTF("[%lld] число которое мы сохранили\n", label_index);
        GETCHAR();

        long long current_instruction_to_jump  = labels_array[label_index];
        potential_error analayze_string_after_label_status = check_string_after_arg(end_pointer); 

        if (analayze_string_after_label_status == OK) {
            PRINTF("после числа ничего нет, всё нормально");
            GETCHAR();
            
            operand_data operand_data_struct = {};
            operand_data_struct.number       = (STACK_DATA_TYPE)current_instruction_to_jump;
            operand_data_struct.flags        = CONST_BIT;

            potential_error write_operand_status = write_operand(func_asm_code,                 operand_data_struct.flags, 
                                                                 operand_data_struct.reg,       operand_data_struct.number, 
                                                                 bytecode, instruction_pointer, bytecode_capacity);

            return write_operand_status;
        }

        else { PRINTF("нашли что-то после числа, строка некорректная\n"); GETCHAR(); return INCORRECT_COMMAND_LINE; }
    }

    else if (*command_line_ptr == '\n') {
        PRINTF("проскипав пробелы дошли до конца строки <=> метки просто нет\n");
        GETCHAR();

        return INCORRECT_COMMAND_LINE;
    }

    else {
        PRINTF("проскипав пробелы мы дошли до символа [%c], что : не является\n", *command_line_ptr);
        GETCHAR();

        return INCORRECT_COMMAND_LINE;
    }
}

potential_error validate_labels_and_else(char*      single_command_line, 
                                         long long* labels_array, 
                                         long long  current_instruction) 
{
    PRINTF("Где я: [%s], [%c]", single_command_line, *single_command_line);
    GETCHAR();

    if (*single_command_line == ':') {
        PRINTF("первый символ двоеточие, всё в порядке\n");
        GETCHAR();

        single_command_line++;
        char*     end_pointer = NULL;
        long long label_index = strtoll(single_command_line, &end_pointer, 10);

        if (end_pointer == single_command_line) {
            PRINTF("число не найдено, end_ptr = command_line_ptr\n");
            GETCHAR();

            return INCORRECT_COMMAND_LINE;
        }

        PRINTF("[%lld] число которое мы сохранили\n", label_index);
        GETCHAR(); // JMP  :102saas

        labels_array[label_index] = current_instruction; // сохранили число типа C которое будет использовано в функции write_command

        potential_error analayze_string_after_label_status = check_string_after_arg(end_pointer); 
        /* вместо функции analyze_string_after_label , которая делает абсолютно ту же самую работу */

        if (analayze_string_after_label_status == OK) {
            PRINTF("после метки ничего нет, всё нормально");
            GETCHAR();

            return OK;
        }

        else { PRINTF("нашли что-то после метки, строка некорректная\n"); GETCHAR(); return INCORRECT_COMMAND_LINE; }
    }

    else if (*single_command_line == '\n') {
        PRINTF("проскипав пробелы дошли до конца строки <=> строчка оказалась пустой\n");
        GETCHAR();

        return EMPTY_STRING;
    }

    else {
        PRINTF("проскипав пробелы мы дошли до символа [%c], что : не является\n", *single_command_line);
        GETCHAR();

        return INCORRECT_COMMAND_LINE;
    }
}

potential_error validate_string_length(char* buffer_in, int debug_index) {
    PRINTF("[%d] debug_index\n", debug_index);
    GETCHAR();

    if (debug_index >= MAX_LINE && buffer_in[debug_index] != '\n' && buffer_in[debug_index] != '\0') {
        PRINTF("переполнение строки; debug_index: [%d]\n", debug_index);
        GETCHAR();

        return STRING_OVERFLOW;
    }

    else { return OK; }
}

uint32_t        pull_the_fucking_command(const char* single_command_line, int* command_length) {
    char command_buffer[MAX_COMMAND_LEN] = {};
    int  sscanf_result = sscanf(single_command_line, "%15s%n", command_buffer, command_length); // MAX_COMMAND_LEN - 1
    // заведомо едим больше чем текущая максимальная длина команды
    
    if (sscanf_result == 1) { 
        uint32_t hash_of_read_command = djb2_calculate_enum_hash(command_buffer);

        return hash_of_read_command;
    }

    else { PRINTF("проблема со сканфом в функции pull_the_fucking_command"); return 0; }
}

void            clear_command_array(char* command) {
    for (int i = 0; i < MAX_LINE; i++) {
        command[i] = 0;
    }
}

void            debugger_like_printf(char* file_buffer, struct stat* to_be_assemblered_struct) {
    for (off_t i = 0; i < to_be_assemblered_struct->st_size + 1; i++) {
        if (file_buffer[i] == '\n') {
            printf("[\\");
            printf("n] ");
        }

        else if (file_buffer[i] == '\0') {
            printf("[\\");
            printf("0] ");
        }

        else { printf("[%c] ", file_buffer[i]); }
    }
    printf("\n");
}