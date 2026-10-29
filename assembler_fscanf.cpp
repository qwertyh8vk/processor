#include "assembler_fscanf.h"
#include "enum_djb.h"

/*
Список текущих тудушек:

    1) обработать все команды кроме MOD в первом и втором валидейте.
    2) прооверка на долбаеба (есть ли лишний аргумент, а именно: там где "1 число на одной строке должно быть одно число, 2 числа на одной строке должно быть два числа.")
    По поводу тудушки два обязательно спросить у ИИ как он бы реализовал это дерьмо на моём месте


    !!!!!!!!!!!!
    // прочитать из памяти в регистр, записать из регистра в память
    // память - это просто массив
    // видеопамять - это тоже прсото массив , но у тебя есть специальная инструкция, какая хочешь сам задавай
    // в процессоре избавиться от копипасты - это макросы, посидеть подумать
    // ХЭШИ!!!!!!!!!!!!! 

*/

int main() {

    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
    // СЛОВЕСНАЯ МЕТКА - ЭТО СТРУКТУРА, КОТОРАЯ СОДЕРЖИТ ИМЯ МЕТКИ И ИНДЕКС, НА КОТОРЫЙ ОНА УКАЗЫВАЕТ. //
    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //

    const char* to_be_assemblered                 = "assembler.txt"; // пока что .txt, потом сделаем бинарщину
    const char* assemblered_file                  = "assembler_res.txt";
    const char* assemblered_file_after_first_asm  = "assembler_res_after_first.txt";

    potential_error obrabotka_faila_status = obrabotka_faila(to_be_assemblered, assemblered_file, assemblered_file_after_first_asm);

    return obrabotka_faila_status;
}

potential_error obrabotka_faila(const char* filename_in, const  char* filename_out, const char* filename_out_after_first_asm) {
    long long labels_array[MAX_LABELS_AMOUNT] = {0};

    for (int i = 0; i < MAX_LABELS_AMOUNT; i++) {
        labels_array[i] = -1;
    }

    size_t          commands_and_arguments_counted = 0;
    potential_error assemble_file_stage_1 = assemble_file_second(filename_in, filename_out, labels_array, FIRST, &commands_and_arguments_counted);
    PRINTF("Ошибка после первого ассембла: [%d]\n", assemble_file_stage_1);
    GETCHAR();
    PRINTF("Количество прочитанных символов: [%zu]\n", commands_and_arguments_counted);
    GETCHAR(); 

    /* идея следующая сделать возвращаемое значение assemble_file_first не enum а long long - количество ячеек с операциями и аргументами.*/
    /* а в assemble_file_second записывать первым числом размер этого файла. можно сделать служебную структурку которая всё будет сверять. */
    /* убираем одну функцию ассемблирования, одну делаем с флагом компиляции просто*/

    potential_error assemble_file_stage_2 = assemble_file_second(filename_in, filename_out, labels_array, SECOND, &commands_and_arguments_counted);
    PRINTF("Ошибка после второго ассембла: [%d]\n", assemble_file_stage_2);
    GETCHAR();

    return OK;
}

potential_error register_processing(FILE* in, FILE* out, const char* func_name, asm_commands_nums func_enum, progon_index progon_index) {
    char reg[MAX_COMMAND_LEN] = {0};

    if (fscanf(in, "%7s", reg) != 1) { 
        fprintf(stderr, "[%s]: нет регистра\n", func_name); 
        
        return NO_REGISTER; 
    }

    int number_of_reg = reg_code(reg);

    if (number_of_reg < 0) {
        fprintf(stderr, "Неизвестный регистр: %s\n", reg); 

        return UNKNOWN_REG; 
    }

    if (progon_index == SECOND) { fprintf(out, "%d %d\n", func_enum, number_of_reg); } 

    return OK;
}

potential_error any_jump_processing(FILE* in, FILE* out, long long* labels_array, 
                                    const char* func_name, asm_commands_nums func_enum, progon_index progon_index) {
    unsigned int label_index = 0;
    char         label[MAX_COMMAND_LEN] = {};

    if (fscanf(in, "%7s", label) != 1) { 
        fprintf(stderr, "[%s]: нет метки\n", func_name);
        
        return NO_LABEL_AFTER_CMD; 
    }

    else {
        char* end_ptr = NULL;
        label_index   = strtol((label + 1 * sizeof(char)), &end_ptr, 10);

        if (label_index > 0 && label_index < MAX_LABELS_AMOUNT - 1) {
            PRINTF("Индекс метки корреткный, допустимый\n");
            PRINTF("Индекc метки в массиве: [%u]\n", label_index);
            GETCHAR();

            if (progon_index == SECOND) { fprintf(out, "%d %lld\n", func_enum, labels_array[label_index]); }// func_enum вместо JUMP_ASM

            return OK;
        }

        else {
            PRINTF("Индекc метки в массиве: [%u]\n", label_index);
            PRINTF("индекс метки некорректный, (выходит за рамки допустимых значений), дропаю ассемблирование\n");
            GETCHAR();

            return INCORRECT_LABEL;
        }
    }
}

potential_error processing_labels(const char* command, long long* labels_array, long long command_index, FILE* out, progon_index progon_index) {
    size_t length_of_command = strlen(command);

    PRINTF("<%s> command, дебажный принт в функции processing_labels\n", command);
    GETCHAR();

    if (length_of_command <= 4 && command[0] == ':') {
        PRINTF("<%s> command, КОМАНДА ПРАВИЛЬНАЯ (ДЛИНА МЕНЬШЕ ИЛИ РАВНА 4 И НАЧИНАЕТСЯ С ДВОЕТОЧИЯ)\n", command);
        GETCHAR();

        char* end_ptr;
        long  label_index_in_array = strtol((command + 1 * sizeof(char)), &end_ptr, 10); // вопрос есть щас узнаем

        if (label_index_in_array > 0 && label_index_in_array < MAX_LABELS_AMOUNT - 1) {
            PRINTF("Индекс метки корреткный, DEFAULT, допустимый\n");
            PRINTF("Label value: [%ld]\n", label_index_in_array);
            GETCHAR();

            labels_array[label_index_in_array] = command_index;

            PRINTF("Я записал в массив меток [%ld] значение [%lld]\n", label_index_in_array, labels_array[label_index_in_array]);
            GETCHAR();

            if      (progon_index == FIRST)  { fprintf(out, "Место метки в массиве: [%ld], метка указывает на индекс массива с номером: [%lld]\n", label_index_in_array, command_index); }
            else if (progon_index == SECOND) { PRINTF("НИЧЕГО НЕ ПЕЧАТАЮ, ВТОРОЙ ПРОГОН\n"); GETCHAR(); }
        }

        else {
            PRINTF("Индекс метки некорректный, выключаю нахуй ассемблирование\n");
            GETCHAR();

            return INCORRECT_LABEL;
        }
    }

    else {
        PRINTF("Ну, поскольку мы в дефолте, и я попал в дефолтовский else, то это точно неизвестная нам команда\n");
        GETCHAR();

        return UNKNOWN_COMMANDLETTE;
    }

    return OK;
}

potential_error push_separate_validating(FILE* in, FILE* out, progon_index progon_index) {
    STACK_DATA_TYPE value = 0;

    if (fscanf(in, "%lf", &value) != 1) { 
        fprintf(stderr, "PUSH: нет аргумента\n"); 
        
        return NO_ARG; 
    }

    if (progon_index == SECOND) { fprintf(out, "%d %lf\n", PUSH_ASM, value); }     

    return OK;
}

potential_error assemble_file_second(const char* filename_in, const char* filename_out, long long* labels_array, 
                                     progon_index progon_index, size_t* commands_and_arguments_counted) {
    assert(filename_in);
    assert(filename_out);

    FILE* in  = fopen(filename_in,  "r");
    FILE* out = fopen(filename_out, "w");

    PRINTF("[%p], [%p]\n", in, out);
    char command[MAX_COMMAND_LEN] = {};
    long long command_index       = 0;
                                        
    if (in != NULL) {
        if (progon_index == SECOND) { fprintf(out, "Service information for CPU: [%zu]\n", *commands_and_arguments_counted); }

        while (fscanf(in, "%7s", command) == 1) {
            uint32_t command_number = djb2_calculate_enum_hash(command);

            PRINTF("=======================================================\n");
            PRINTF("<%u> как я это запаковал, " 
                   "<%s> что это\n", command_number, command);
            PRINTF("=======================================================\n");
            GETCHAR();

            switch (command_number) {
                case PUSH: { // два аргумента
                    potential_error error_aft_push_validating = push_separate_validating(in, out, progon_index);     
                    if (error_aft_push_validating != OK) { return error_aft_push_validating; }    
                    command_index += 2;      
                    break; 
                }

                MOKRIY_CASE_NOFUNC(POP);
                MOKRIY_CASE_NOFUNC(ADD);
                MOKRIY_CASE_NOFUNC(SUB);
                MOKRIY_CASE_NOFUNC(DIV);
                MOKRIY_CASE_NOFUNC(MUL);
                MOKRIY_CASE_NOFUNC(IN);
                MOKRIY_CASE_NOFUNC(SQRT);
                MOKRIY_CASE_NOFUNC(RET);   
                MOKRIY_CASE_NOFUNC(OUT);
                MOKRIY_CASE_NOFUNC(HLT);
                MOKRIY_CASE_NOFUNC(DUP);
                MOKRIY_CASE_NOFUNC(CMP);
                MOKRIY_CASE_FUNC(PSHR,  register_processing(in, out, "PSHR", PSHR_ASM, progon_index));
                MOKRIY_CASE_FUNC(POPR,  register_processing(in, out, "POPR", POPR_ASM, progon_index));
                MOKRIY_CASE_FUNC(JMPA,  any_jump_processing(in, out, labels_array, "JMPA", JMPA_ASM, progon_index));
                MOKRIY_CASE_FUNC(JMPB,  any_jump_processing(in, out, labels_array, "JMPB", JMPB_ASM, progon_index));
                MOKRIY_CASE_FUNC(JMPE,  any_jump_processing(in, out, labels_array, "JMPE", JMPE_ASM, progon_index));
                MOKRIY_CASE_FUNC(JAE,   any_jump_processing(in, out, labels_array, "JAE",  JAE_ASM,  progon_index));
                MOKRIY_CASE_FUNC(JBE,   any_jump_processing(in, out, labels_array, "JBE",  JBE_ASM,  progon_index));
                MOKRIY_CASE_FUNC(JMP,   any_jump_processing(in, out, labels_array, "JMP",  JMP_ASM,  progon_index));
                MOKRIY_CASE_FUNC(CALL,  any_jump_processing(in, out, labels_array, "CALL", CALL_ASM, progon_index));

                default: { /* если встретили метку */
                    potential_error status_after_default_val = processing_labels(command, labels_array, command_index, out, progon_index);
                    if (status_after_default_val != OK) { return status_after_default_val; }
                    break;
                }
            }

            clear_command_array(command);
        }

        if (progon_index == FIRST) { *commands_and_arguments_counted = (command_index + 1); }

        fclose(in);  // MANDATORY!!!!! нахуй
        fclose(out); // MANDATORY!!!!! нахуй
    }

    else {
        PRINTF("не удалось открыть файл\n");
        GETCHAR();

        return FILE_NOT_OPENNED;
    }

    return OK;
}

int reg_code(const char *name) {

    if (strcmp(name, "AX") == 0) return 0;
    if (strcmp(name, "BX") == 0) return 1;
    if (strcmp(name, "CX") == 0) return 2;
    if (strcmp(name, "DX") == 0) return 3;
    
    return -1;
}

void clear_command_array(char* command) {
    for (int i = 0; i < MAX_COMMAND_LEN; i++) {
        command[i] = 0;
    }
}