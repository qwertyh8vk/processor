#include "processor.h"

/*
0 0  0 -1
0 0  1 0
1 0 -4 (2 -2 +2)
1 0  4  0
1 0  0  (1 0 || 2 0 0)
0 5  10 (1 -2)
*/

int main() {
    const char* assembler_to_cpu = "assembler_res.bin";

    //NOTE - тут создание асм файла для кружка
    const char* circle_assembler = "circle_assembler.txt";

    // create_asm_for_circle(circle_assembler, 15);

    size_t bytes_in_bin_file_amount = get_bytes_in_bin_amount(assembler_to_cpu);

    PROCPRINTF("Что насчитала моя функция get_bytes_in_bin_amount: [%zu]\n", bytes_in_bin_file_amount);
    PROCGETCHAR();

    cpu_struct* ryzen7_5700 = initialize_cpu_struct(assembler_to_cpu, bytes_in_bin_file_amount);

    cpu_stack_dump(ryzen7_5700, CALL_STACK);
    cpu_stack_dump(ryzen7_5700, CALC_STACK);

    cpu_work_cycle(ryzen7_5700);

    return OK;
}

#include "stack_v3_ot.h"

struct cpu_struct {
    uint8_t*          code;

    long long         instruction_pointer;
    size_t            code_obsolet_size;

    defended_stack*   calculations_stack;
    defended_stack*   call_stack;

    STACK_DATA_TYPE   regs[N_REGS];
    cpu_status        processor_status;

    cmp_compare_res   cmp_flag;
    STACK_DATA_TYPE   videoram[RAM_MB];
};
 
// TODO  - Теперь массив кода - uint8_t неравномерный. переделать массив кода
// TODO  - переделать в каждой функции переход instruction pointer
// TODO  - обработка всех команд кроме push и pop
// TODO  - обработка полей push / pop

// TODO  - видеопамять
// TODO  - Фибоначи
// TODO  - квадратка

// FIXME - [-Wsign-compare] !!!!!!!!!!!!!!!!!

cpu_status  push_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);
    PROCGETCHAR();

    uint8_t our_command = (cpu->code)[cpu->instruction_pointer];
    cpu->instruction_pointer += sizeof(uint8_t);

    PROCPRINTF("что у нас там по команде" RED " (десятичное представление): " RESET "<%u>\n", our_command);
    PROCGETCHAR();

    STACK_DATA_TYPE number = 0;

    if (!(our_command ^ ONLY_RAM) || !(our_command ^ NOTHING)) {
        PROCPRINTF("По-хорошему это должно быть работой ассемблера. попали в недопустимый режим для пуша\n");

        return BAD_COMMAND_MODE;
    }

    if (our_command & REGISTER_BIT) { 
        uint8_t our_register = (cpu->code)[cpu->instruction_pointer];
        PROCPRINTF("our register: <%u>\n", our_register);
        PROCGETCHAR();
        assert((our_register > 0) && (our_register < 5));

        number += (cpu->regs)[our_register - 1]; // потому что записывались enum'ы от 1 до 4. 
        cpu->instruction_pointer += sizeof(uint8_t);
    }

    if (our_command & CONST_BIT) {
        STACK_DATA_TYPE chislo = 0;
        memcpy(&chislo, &((cpu->code)[cpu->instruction_pointer]), sizeof(STACK_DATA_TYPE));

        PROCPRINTF("число которое скопировали: <%lf>\n", chislo);
        PROCGETCHAR();

        number += chislo;
        cpu->instruction_pointer += sizeof(STACK_DATA_TYPE);
    }

    if (our_command & MEMORY_BIT) {
        cpu_status check_whole_number_status = check_whole_number(number);

        if (check_whole_number_status == CPU_FINE) {
            cpu_status validate_videoram_index_status = validate_videoram_index(number);

            if (validate_videoram_index_status == CPU_FINE) {
                size_t index_to_address = (size_t)(round(number));
                number = (cpu->videoram)[index_to_address];

                // NOTE - намбер стал числом, которое лежало в видеопамяти по вычисленному индексу.
            }

            else { return validate_videoram_index_status; }
        }

        else { return check_whole_number_status; }
    }

    PROCPRINTF("Без отладки никуда. что мы считали: [%lf]\n", number);

    defended_stack* ptr_on_cpu_calc_stack   = cpu->calculations_stack;
    stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, number);

    return finish_instruction(cpu, stack_push_status, 0);
}

cpu_status  pop_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    uint8_t our_command = (cpu->code)[cpu->instruction_pointer];
    defended_stack* ptr_on_cpu_calc_stack  = cpu->calculations_stack;

    cpu->instruction_pointer += sizeof(uint8_t);

    STACK_DATA_TYPE videoram_index = 0;

    if (!(our_command ^ ONLY_RAM) || !(our_command ^ REG_CONST) || !(our_command ^ CONST)) {
        PROCPRINTF("По-хорошему это должно быть работой ассемблера. попали в недопустимый режим для попа\n");
        PROCGETCHAR();

        return BAD_COMMAND_MODE;
    }

    if (our_command & MEMORY_BIT) {
        if (our_command & REGISTER_BIT) {
            uint8_t our_register = (cpu->code)[cpu->instruction_pointer];

            assert((our_register > 0) && (our_register < 5));

            STACK_DATA_TYPE value_laying_in_register = (cpu->regs)[our_register - 1]; 
            // NOTE - потому что записывались enum'ы от 1 до 4. 
            videoram_index += value_laying_in_register; 
            cpu->instruction_pointer += sizeof(uint8_t);
        }

        if (our_command & CONST_BIT) {
            STACK_DATA_TYPE chislo = 0;
            memcpy(&chislo, &((cpu->code)[cpu->instruction_pointer]), sizeof(STACK_DATA_TYPE));

            PROCPRINTF("число которое скопировали: <%lf>\n", chislo);
            PROCGETCHAR();

            videoram_index += chislo;
            cpu->instruction_pointer += sizeof(STACK_DATA_TYPE);
        }

        cpu_status check_whole_number_status = check_whole_number(videoram_index);
        if (check_whole_number_status != CPU_FINE) { return check_whole_number_status; }
        
        cpu_status validate_videoram_index_status = validate_videoram_index(videoram_index);
        if (validate_videoram_index_status  != CPU_FINE) { return validate_videoram_index_status; }

        stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, &((cpu->videoram)[(size_t)videoram_index]));
        // NOTE - две проверки подряд - на валидность по индексу видеопамяти и целость числа

        return finish_instruction(cpu, stack_pop_status, 0);
    }

    if (our_command & REGISTER_BIT) {
        uint8_t our_register = (cpu->code)[cpu->instruction_pointer];

        assert((our_register > 0) && (our_register < 5));

        stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, &((cpu->regs)[our_register - 1]));
        return finish_instruction(cpu, stack_pop_status, sizeof(uint8_t));
    }

    else {
        PROCPRINTF(BLUE "ПУСТОЙ ПОП!\n" RESET);
        PROCGETCHAR();

        stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, NULL);
        return finish_instruction(cpu, stack_pop_status, 0);
    }
}

// ----------------------------------------------------------------------------------------------------------------

cpu_status  add_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;
    STACK_DATA_TYPE value_b = 0;

    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    stack_potential_error stack_pop_a_status = stack_pop(ptr_on_cpu_calc_stack, &value_a);
    stack_potential_error stack_pop_b_status = stack_pop(ptr_on_cpu_calc_stack, &value_b);

    if ((stack_pop_a_status == OK) && (stack_pop_b_status == OK)) {
        STACK_DATA_TYPE sum_of_a_b = value_a + value_b;

        stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, sum_of_a_b);

        return finish_instruction(cpu, stack_push_status, 1);
    }

    else { return CPU_ERR; }
}

cpu_status  sub_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;
    STACK_DATA_TYPE value_b = 0;
    /* 
    5
    3
    */
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;
    
    stack_potential_error stack_pop_a_status = stack_pop(ptr_on_cpu_calc_stack, &value_a); // 5
    stack_potential_error stack_pop_b_status = stack_pop(ptr_on_cpu_calc_stack, &value_b); // 3

    if ((stack_pop_a_status == OK) && (stack_pop_b_status == OK)) {
        STACK_DATA_TYPE diff_betw_a_b = value_a - value_b; // 5 - 3

        stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, diff_betw_a_b);

        return finish_instruction(cpu, stack_push_status, 1);
    }

    else { return CPU_ERR; }
}

// MACROS_FOR_SUB_MUL_DIV(cpu, +)

cpu_status  mul_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;
    STACK_DATA_TYPE value_b = 0;

    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    stack_potential_error stack_pop_a_status = stack_pop(ptr_on_cpu_calc_stack, &value_a);
    stack_potential_error stack_pop_b_status = stack_pop(ptr_on_cpu_calc_stack, &value_b);

    if ((stack_pop_a_status == OK) && (stack_pop_b_status == OK)) {
        STACK_DATA_TYPE mul_of_a_b = value_a * value_b;

        stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, mul_of_a_b); 

        return finish_instruction(cpu, stack_push_status, sizeof(uint8_t));
    }
    
    else { return CPU_ERR; }
}

cpu_status  div_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;
    STACK_DATA_TYPE value_b = 0;

    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    stack_potential_error stack_pop_a_status = stack_pop(ptr_on_cpu_calc_stack, &value_a);
    stack_potential_error stack_pop_b_status = stack_pop(ptr_on_cpu_calc_stack, &value_b);

    if ((stack_pop_a_status == OK) && (stack_pop_b_status == OK)) {
        STACK_DATA_TYPE mul_of_a_b = 0;

        if (fabs(value_b - 0) > EPSILON) { mul_of_a_b = value_a / value_b; }

        else {
            PROCPRINTF("Деление на ноль я пока ещё не определил. мб выкачу в дальнейших обновлениях\n");
            PROCGETCHAR();

            return CPU_ERR;
        }

        stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, mul_of_a_b);

        return finish_instruction(cpu, stack_push_status, sizeof(uint8_t));
    }

    else { return CPU_ERR; }
}

cpu_status  sqrt_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;

    defended_stack*  ptr_on_cpu_calc_stack = cpu->calculations_stack;
    stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, &value_a);

    if (stack_pop_status == OK) {
        if (value_a >= 0) {
            STACK_DATA_TYPE sqrt_a = sqrt(value_a); 

            stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, sqrt_a);

            return finish_instruction(cpu, stack_push_status, sizeof(uint8_t));
        }

        else {
            PROCPRINTF("Корень квадратный из отр. числа пока не определил. мб выкачу в дальнейших обновлениях\n");
            PROCGETCHAR();

            return CPU_ERR;
        }
    }

    else { return CPU_ERR;  }
}

cpu_status  mod_asm(cpu_struct* cpu) {
    return CPU_FINE; // пустая функция пока что
}

// ----------------------------------------------------------------------------------------------------------------

cpu_status  jmp_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    cpu->instruction_pointer += sizeof(uint8_t);
    long long where_we_want_to_jump = 0;

    memcpy(&where_we_want_to_jump, &((cpu->code)[cpu->instruction_pointer]), sizeof(long long));
    PROCPRINTF("Что сохранилось (jmp_asm): <%lld>\n", where_we_want_to_jump);

    const size_t code_obsolet_size = cpu->code_obsolet_size;

    if (where_we_want_to_jump >= 0 && where_we_want_to_jump < (long long)code_obsolet_size) { cpu->instruction_pointer = where_we_want_to_jump; }
    
    else {
        PROCPRINTF("Нубля сюда прыгать я ещё не научился\n");
        PROCGETCHAR();

        return CPU_ERR;
    }

    PROCPRINTF("Установил точку откуда будем выполнять команды\n");
    cpu_code_array_dump(cpu, code_obsolet_size);

    return CPU_FINE;
}

cpu_status  jmp_cond_universal(cpu_struct* cpu, 
                               cmp_compare_res mode_1, 
                               cmp_compare_res mode_2) 
{
    PROCPRINTF(PURPLE "ВЫЗВАННЫЙ РЕЖИМ ИМЕЕТ КОД [%d] [%d]\n" RESET, mode_1, mode_2); // можно переделать в таблицу кодов , для этого нужно завести структуру

    /* //////////////
    /               /
    /  EQUAL ABOVE  /
    /  EQUAL BELOW  /
    /               <<<--------------- Возможные режимы условных сравнений бл
    /  ABOVE ABOVE  /
    /  BELOW BELOW  /
    /  EQUAL EQUAL  /
    /               /
    */ //////////////

    PROCGETCHAR();

    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);
    cpu->instruction_pointer += sizeof(uint8_t);

    long long where_we_want_to_jump = 0;
    memcpy(&where_we_want_to_jump, &((cpu->code)[cpu->instruction_pointer]), sizeof(long long));
    const size_t code_obsolet_size = cpu->code_obsolet_size;

    PROCPRINTF("куда мы хотим прыгнуть (на какой индекс): [%lld]\n", where_we_want_to_jump);
    PROCGETCHAR();

    int cpu_current_flag = cpu->cmp_flag;

    if (cpu_current_flag == mode_1 || cpu_current_flag == mode_2) {
        if (where_we_want_to_jump >= 0 && where_we_want_to_jump < (long long)code_obsolet_size) { 
            cpu->instruction_pointer = where_we_want_to_jump; 
            PROCPRINTF("Установил точку откуда будем выполнять команды\n");
            cpu_code_array_dump(cpu, code_obsolet_size);

            return CPU_FINE;
        }

        else {
            PROCPRINTF("Нубля сюда прыгать я ещё не научился\n");
            PROCGETCHAR();

            return CPU_ERR;
        }
    } // one more КОПИПАСТ, который можно починить

    else {
        PROCPRINTF("Ластовый вызванный CMP имеет значение не равное выбранному режиму: cmp_flag value = [%d]\n", cpu_current_flag);
        PROCGETCHAR();

        cpu->instruction_pointer += sizeof(long long);
        return CPU_FINE;
    }
}

JMP_MACROS_INITIALIZE(jmpa, ABOVE, ABOVE);
JMP_MACROS_INITIALIZE(jmpb, BELOW, BELOW);
JMP_MACROS_INITIALIZE(jae,  EQUAL, ABOVE);
JMP_MACROS_INITIALIZE(jbe,  EQUAL, BELOW);
JMP_MACROS_INITIALIZE(jmpe, EQUAL, EQUAL);

// ----------------------------------------------------------------------------------------------------------------

cpu_status  call_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    defended_stack* ptr_on_cpu_call_stack = cpu->call_stack;
    cpu->instruction_pointer += sizeof(uint8_t);

    long long where_we_want_to_jump = 0;
    memcpy(&where_we_want_to_jump, &((cpu->code)[cpu->instruction_pointer]), (long long)sizeof(long long));

    long long where_we_want_to_start_from_again = cpu->instruction_pointer + (long long)sizeof(long long);
    const size_t code_obsolet_size = cpu->code_obsolet_size;

    PROCPRINTF("Значение, которое возьмёт ret в качестве ориентира куда возвращаться: [%lld]\n", where_we_want_to_start_from_again);
    PROCGETCHAR();

    stack_potential_error stack_push_wwwtsf_status = BAD_DEFAULT;

    if (where_we_want_to_start_from_again >= 0 && where_we_want_to_start_from_again < (long long)code_obsolet_size) {
        stack_push_wwwtsf_status = stack_push(ptr_on_cpu_call_stack, (STACK_DATA_TYPE)where_we_want_to_start_from_again); // NOTE - приходится приводить к double, поскольку 
        stack_dump(ptr_on_cpu_call_stack);
    }

    else {
        PROCPRINTF(DARK_PURPLE "Походу после CALL ничего не идёт, в таком случае ничего не делаю " RED "(Относим это к ошибке)\n" RESET);
        PROCGETCHAR();

        return CPU_ERR;
    }
    
    if (stack_push_wwwtsf_status == OK) {
        if (where_we_want_to_jump >= 0 && where_we_want_to_jump < (long long)code_obsolet_size) { 
            cpu_code_array_dump(cpu, code_obsolet_size);

            cpu->instruction_pointer = where_we_want_to_jump; 
            PROCPRINTF("Установил точку откуда будем выполнять команды\n");

            cpu_code_array_dump(cpu, code_obsolet_size);

            return CPU_FINE;
        }

        else {
            PROCPRINTF("Нубля сюда прыгать я ещё не научился\n");
            PROCGETCHAR();

            return CPU_ERR;
        }
    }

    else { return CPU_ERR; }
}

cpu_status  ret_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    defended_stack* ptr_on_cpu_call_stack = cpu->call_stack;
    STACK_DATA_TYPE where_we_want_to_start_from_again = 0;

    stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_call_stack, &where_we_want_to_start_from_again);
    if (stack_pop_status == OK) {
        stack_dump(ptr_on_cpu_call_stack);

        PROCPRINTF("Индекс который сейчас вытащил ret: [%lld]\n", (long long)where_we_want_to_start_from_again);

        cpu_code_array_dump(cpu, cpu->code_obsolet_size);

        cpu->instruction_pointer = (long long)where_we_want_to_start_from_again;
        PROCPRINTF("Установил нужный индекс в ret'e\n");
        PROCGETCHAR();

        cpu_code_array_dump(cpu, cpu->code_obsolet_size);

        return CPU_FINE;
    }

    else { return CPU_ERR; }
}

// ----------------------------------------------------------------------------------------------------------------

cpu_status  in_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);
    printf("НУ что, товарищ пользователь, введите число которое я запушу в calc_stack: ");
    char  read_string_typed_by_user[MAX_AMT_OF_DGTS] = {};
    char* fgets_result = fgets(read_string_typed_by_user, MAX_AMT_OF_DGTS, stdin);

    if (fgets_result != NULL) {
        PROCPRINTF("успешно записана строка в массив read_string_typed_by_user\n");
        PROCGETCHAR();

        defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

        char* end_ptr = NULL;
        STACK_DATA_TYPE value_to_push_from_stdin = strtod(read_string_typed_by_user, &end_ptr);

        if (*end_ptr == '\n') {
            PROCPRINTF("На каком символе закончилось преобразование strtod: [\\");
            PROCPRINTF("n]\n");
            PROCGETCHAR();
        }

        else {
            PROCPRINTF("На каком символе закончилось преобразование strtod: [%c]", *end_ptr);
            PROCGETCHAR();
        }

        PROCPRINTF(PURPLE "Что считали: <%lf>\n" RESET, value_to_push_from_stdin);
        PROCGETCHAR();
        stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, value_to_push_from_stdin);

        return finish_instruction(cpu, stack_push_status, sizeof(uint8_t));
    }

    else {
        PROCPRINTF("fgets_result вернул NULL, нихуя себе!\n");
        PROCGETCHAR();

        return CPU_ERR;
    }
}

cpu_status  out_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_to_out = 0;
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    cpu_status      stack_dup_status = dup_asm(cpu);

    if (stack_dup_status == CPU_FINE) {
        PROCPRINTF(GREEN "Успешно " RESET "сдублировал число\n");
        PROCGETCHAR();

        stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, &value_to_out);
        if (stack_pop_status == OK) {
            
            printf(GREEN "ЛАСТ ЧИСЛО ИЗ СТЭКА: " RESET PURPLE "[%lf]\n" RESET, value_to_out);

            // cpu->instruction_pointer += sizeof(uint8_t);
            // NOTE - здесь не нужен шаг +1 поскольку мы его сделали внутри dup_asm

            return CPU_FINE;
        }

        else { return CPU_ERR; }
    }

    else { return CPU_ERR; }
}

cpu_status  hlt_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: <%lld>\n", cpu->instruction_pointer);

    return CPU_HLT;
}

// ----------------------------------------------------------------------------------------------------------------

cpu_status  dup_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_to_dup = 0;
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    

    stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, &value_to_dup);

    if (stack_pop_status == OK) {
        PROCPRINTF("Что я сохранил в value_to_dup из стэка: [%lf]\n", value_to_dup);
        PROCGETCHAR();
        
        stack_potential_error stack_push_first_status  = stack_push(ptr_on_cpu_calc_stack, value_to_dup);
        stack_potential_error stack_push_second_status = stack_push(ptr_on_cpu_calc_stack, value_to_dup);

        
        // cpu_status err_code = finish_instruction(cpu, stack_push_first_status, 0); 

        if (stack_push_first_status != OK) { return CPU_ERR; }
        else { 
            
            return finish_instruction(cpu, stack_push_second_status, sizeof(uint8_t)); 
        }

    }

    else { return CPU_ERR; }
}

cpu_status  cmp_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    STACK_DATA_TYPE value_a_to_cmp_from_stack = 0; // 5
    STACK_DATA_TYPE value_b_to_cmp_from_stack = 0; // 3

    /*
    5
    3  ---> CMP 5 3
    */

    stack_potential_error stack_pop_status_a = stack_pop(ptr_on_cpu_calc_stack, &value_a_to_cmp_from_stack);
    stack_potential_error stack_pop_status_b = stack_pop(ptr_on_cpu_calc_stack, &value_b_to_cmp_from_stack);

    if (stack_pop_status_a == OK && stack_pop_status_b == OK) {
        

        PROCPRINTF(YELLOW "Вот что я сохранил в value_a: [%lf]\n", value_a_to_cmp_from_stack);
        PROCPRINTF("Вот что я сохранил в value_b: [%lf]\n" RESET,  value_b_to_cmp_from_stack);
        PROCGETCHAR();
        
        if      (value_a_to_cmp_from_stack > value_b_to_cmp_from_stack) { cpu->cmp_flag = ABOVE; }
        else if (value_a_to_cmp_from_stack < value_b_to_cmp_from_stack) { cpu->cmp_flag = BELOW; }
        else                                                            { cpu->cmp_flag = EQUAL; }

        stack_potential_error stack_push_b_status = stack_push(ptr_on_cpu_calc_stack, value_b_to_cmp_from_stack);
        stack_potential_error stack_push_a_status = stack_push(ptr_on_cpu_calc_stack, value_a_to_cmp_from_stack);

        if (stack_push_b_status == OK && stack_push_a_status == OK) { 
            
            PROCPRINTF("Запушил два числа с сохранением порядка\n");
            PROCGETCHAR();

            cpu->instruction_pointer += sizeof(uint8_t);

            return CPU_FINE;
        }

        else { return CPU_ERR; }
    }

    else { return CPU_ERR; }
}

cpu_status  over_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;
    STACK_DATA_TYPE value_b = 0;
    /* 
    1 - последний
    2 - предпоследний
    */
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;
    
    stack_potential_error stack_pop_a_status = stack_pop(ptr_on_cpu_calc_stack, &value_a); // 1
    stack_potential_error stack_pop_b_status = stack_pop(ptr_on_cpu_calc_stack, &value_b); // 2

    if ((stack_pop_a_status == OK) && (stack_pop_b_status == OK)) {
        stack_potential_error stack_push_1_status = stack_push(ptr_on_cpu_calc_stack, value_b);
        stack_potential_error stack_push_2_status = stack_push(ptr_on_cpu_calc_stack, value_a);
        stack_potential_error stack_push_3_status = stack_push(ptr_on_cpu_calc_stack, value_b);

        if (stack_push_1_status != OK) { return CPU_ERR; }
        if (stack_push_2_status != OK) { return CPU_ERR; }

        return finish_instruction(cpu, stack_push_3_status, 1);
    }

    else { return CPU_ERR; }
}

cpu_status  swap_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;
    STACK_DATA_TYPE value_b = 0;
    /* 
    5
    3
    */
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;
    
    stack_potential_error stack_pop_a_status = stack_pop(ptr_on_cpu_calc_stack, &value_a); // 5
    stack_potential_error stack_pop_b_status = stack_pop(ptr_on_cpu_calc_stack, &value_b); // 3

    if ((stack_pop_a_status == OK) && (stack_pop_b_status == OK)) {
        stack_potential_error stack_push_1_status = stack_push(ptr_on_cpu_calc_stack, value_a);
        stack_potential_error stack_push_2_status = stack_push(ptr_on_cpu_calc_stack, value_b);

        if (stack_push_1_status != OK) { return CPU_ERR; }

        return finish_instruction(cpu, stack_push_2_status, 1);
    }

    else { return CPU_ERR; }
}

cpu_status  dspl_asm(cpu_struct* cpu) {
    PROCPRINTF("Текущий байт байткода: [%lld]\n", cpu->instruction_pointer);

    for (int i = 0; i < RAM_MB; i++) {
        if ((i % ARR_HEIGHT) == ARR_HEIGHT - 1) {
            if (fabs((cpu->videoram)[i] - 1) < EPSILON) {
                printf("##\n");
            }

            else {
                printf("  \n");
            }
        }
        
        else {
            if (fabs((cpu->videoram)[i] - 1) < EPSILON) {
                printf("##");
            }

            else {
                printf("  ");
            }
        }
    }

    cpu->instruction_pointer += sizeof(uint8_t);
    
    return CPU_FINE;
}
// FIXME - много копипасты у следующих функций:
// FIXME - add, sub, mul, div
// FIXME - push, over, swap

// ----------------------------------------------------------------------------------------------------------------

cpu_status  finish_instruction(cpu_struct* cpu, stack_potential_error stack_opname_status, long long bytes_amount) {
    if (stack_opname_status == OK) { 
        cpu->instruction_pointer += bytes_amount;

        cpu_code_array_dump(cpu, cpu->code_obsolet_size); 
        return CPU_FINE; 
    }

    else { return CPU_ERR; }
}

cpu_struct* initialize_cpu_struct(const char* filename, size_t bytes_read_from_bin) {
    assert(filename);

    int file_descr = open(filename, O_RDONLY);

    if (file_descr > 0) { // если файл открылся
        cpu_struct* ryzen7_5700 = (cpu_struct *)calloc(1, sizeof(cpu_struct)); // создали структурку

        ryzen7_5700->code = (uint8_t *)calloc(bytes_read_from_bin, sizeof(uint8_t)); // первая переделка - массив кода теперь имеет тип uint8_t
        // NOTE - Первый шаг положен, двигаемся дальше

        ssize_t bytes_written_in_buffer = read(file_descr, ryzen7_5700->code, bytes_read_from_bin);

        if (bytes_written_in_buffer == bytes_read_from_bin) {
            ryzen7_5700->instruction_pointer = 0; // по умолчанию указываем на первый байт в массиве code
            ryzen7_5700->code_obsolet_size   = bytes_read_from_bin;
            ryzen7_5700->calculations_stack  = stack_init(BASIC_LENGTH);
            ryzen7_5700->call_stack          = stack_init(BASIC_LENGTH);
            ryzen7_5700->processor_status    = CPU_FINE;
            ryzen7_5700->cmp_flag            = EQUAL; // 0 по дефолту, что достаточно логично
            
            for (int i = 0; i < N_REGS; i++) {
                (ryzen7_5700->regs)[i] = 0; // значения всех регистров по умолчанию выставляем в ноль.
            }

            for (int i = 0; i < RAM_MB; i++) {
                (ryzen7_5700->videoram)[i] = 0;
            }

            return ryzen7_5700;
        }

        else {
            PROCPRINTF("говорит функция initialize_cpu_struct!\nМы блять недосчитались команд или аргументов!\n");

            return NULL;
        }
    }

    else {
        PROCPRINTF("говорит функция initialize_cpu_struct!\nфайл блять не открылся!\n");

        return NULL;
    }
}

size_t      get_bytes_in_bin_amount(const char* filename) {
    assert(filename);

    struct stat file_info;

    if (stat(filename, &file_info) == 0) {
        off_t file_size = file_info.st_size;
        PROCPRINTF("[%lld], file_size\n", file_size);
        PROCGETCHAR();

        if (file_size > 0) { return (size_t)file_size; }

        else { 
            PROCPRINTF("filesize <= 0 (функция get_bytes_in_bin_amount)\n"); 
            PROCGETCHAR(); 
            
            return 0; 
        }
    }

    return FILE_WAS_NOT_OPPENED_PROPERLY;
}

cpu_status  cpu_work_cycle(cpu_struct* cpu) {
    const size_t code_obsolet_size = cpu->code_obsolet_size;

    /* условия выхода: адекватное - мы дошли до HLT, или мы дошли до конца кода, но при этом HLT не встретили - в таком случае мы шлём пользователя нахуй и дропаем программу!*/
    while(cpu->instruction_pointer >= 0 && cpu->instruction_pointer < (long long)code_obsolet_size) {
        /* пока работаем - исполняй команды. обрабатывай статус после каждой команды */
        
        uint8_t what_we_are_executing = (cpu->code)[cpu->instruction_pointer];

        // NOTE - каждый раз начинаю с КОМАНДЫ, 1 это байт ансайн чар

        PROCPRINTF("<%u> что я только что вытащил?" PURPLE " UNSIGNED\n" RESET, what_we_are_executing);
        PROCGETCHAR();

        uint8_t our_current_opcode = what_we_are_executing & OPCODE_MASKA;

        PROCPRINTF(GREEN "OPCODE" RESET " который полулилось вытащить: <%u>\n", our_current_opcode);
        PROCGETCHAR();

        uint8_t opcode_index_in_array = our_current_opcode - 1; // минимальный our_current_opcode равен единице, поэтому index_in_array при нормальной работе должен быть равен 0

        if (opcode_index_in_array >= 0 && opcode_index_in_array <= N_CMDS) {
            PROCPRINTF("Всё нормально! " ORANGE "(cpu_work_cycle)\n" RESET);
            PROCGETCHAR();

            cpu_status renew_status_after_command = asm_funcs[opcode_index_in_array](cpu);

            if (renew_status_after_command == CPU_HLT) {
                PROCPRINTF(GREEN "Мы дошли до хальт. Ниче стращного щас просто ебнем програмку и всё.\n" RESET);

                cpu->processor_status = CPU_HLT; // в структурку всё таки это надо записать
                return CPU_HLT;
            }

            else if (renew_status_after_command == CPU_ERR) {
                PROCPRINTF("ПРОИЗОШЛА ОШИБКА! ПИЗДЕЦ! Да шучу я. всё фиксится. Код функции, после которой произошел бум: [%lld]\n", cpu->instruction_pointer);

                cpu->processor_status = CPU_ERR;
                return CPU_ERR;
            }

            else if (cpu->instruction_pointer >= (long long)code_obsolet_size) {
                PROCPRINTF(ORANGE "Мы НЕ дошли до хальт. Ниче стращного щас просто ебнем програмку ВМЕСТЕ С ОПЕРАЦИОНКОЙ и всё.\n" RESET);

                cpu->processor_status = CPU_DESTROY_USER_SYSTEM; // в структурку всё таки это надо записать
                return CPU_DESTROY_USER_SYSTEM;
            }

            cpu_stack_dump(cpu, CALC_STACK);
        }

        else {
            PROCPRINTF("Тут какие варианты. 99 процентов что цикл работает неправильно (виноват разработчик)," 
                   "и 1 процент что мы реально наткнулись на неизвестную команду\n");

            cpu->processor_status = CPU_UNK_CMD;
            return CPU_UNK_CMD;
        }
    }

    PROCPRINTF("До сюда мы не должны были дойти, поэтому это необработанный случай\n");
    cpu->processor_status = CPU_UNCHECKED_CASE;

    return CPU_UNCHECKED_CASE;
}

cpu_status  check_whole_number(STACK_DATA_TYPE number) {
    STACK_DATA_TYPE number_rounded = round(number);

    if (fabs(number - number_rounded) > EPSILON) {
        PROCPRINTF("От округленного числа number отличается больше, чем на EPSILON\n");
        PROCGETCHAR();

        return NUMBER_IS_NOT_WHORE;
    }

    return CPU_FINE;
}

cpu_status  validate_videoram_index(STACK_DATA_TYPE number) {
    if (number >= 0 && number <= RAM_MB) {
        PROCPRINTF("индекс допустимый\n");
        PROCGETCHAR();

        return CPU_FINE;
    }

    else {
        PROCPRINTF("validate_videoram_index - индекс недопустимый для адрессации к массиву видеопамяти: <%lf>\n", number);
        
        return INVALID_VIDEORAM_INDEX;
    }
}

void        cpu_code_array_dump(cpu_struct* cpu, size_t size_of_array) {
    assert(cpu);

    PROCPRINTF(DARK_PURPLE "========================================================================================================================\n");

    PROCPRINTF("CPU CURR INST: [%lld]\n", cpu->instruction_pointer);

    long long j = 0;

    for (long long i = 0; i < size_of_array; i++) {
        if (j % 16 == 15) {
            if (i == cpu->instruction_pointer) { PROCPRINTF(PURPLE "<%u>\n" RESET, (cpu->code)[i]); }
            else                               { PROCPRINTF(DARK_PURPLE "<%u>\n", (cpu->code)[i]);  }
        }

        else {
            if ((cpu->code)[i] > 99) {
                if (i == cpu->instruction_pointer) { PROCPRINTF(PURPLE "<%u>  " RESET, (cpu->code)[i]); }
                else                               { PROCPRINTF(DARK_PURPLE "<%u>  ", (cpu->code)[i]);  }
            }

            else if ((cpu->code)[i] > 9) {
                if (i == cpu->instruction_pointer) { PROCPRINTF(PURPLE "<%u>   " RESET, (cpu->code)[i]); }
                else                               { PROCPRINTF(DARK_PURPLE "<%u>   ", (cpu->code)[i]);  }
            }

            else {
                if (i == cpu->instruction_pointer) { PROCPRINTF(PURPLE "<%u>    " RESET, (cpu->code)[i]); }
                else                               { PROCPRINTF(DARK_PURPLE "<%u>    ", (cpu->code)[i]);  }
            }
        }

        j++;
    }

    PROCPRINTF("\n");
    PROCPRINTF(DARK_PURPLE "========================================================================================================================\n" RESET);
}

void        create_asm_for_circle(const char* filename, int circle_radius) {
    FILE* file_pointer = fopen(filename, "w");

    if (file_pointer != NULL) {
        int x0             = (ARR_WIDTH  - 1) / 2;   // 27
        int y0             = (ARR_HEIGHT - 1) / 2;   // 27
        int squared_radius = circle_radius * circle_radius;

        for (int y = 0; y < ARR_HEIGHT; y++) {
            for (int x = 0; x < ARR_WIDTH; x++) {
                int dx = x - x0;
                int dy = y - y0;

                int element_coords = x + y * ARR_WIDTH;

                if ((dx * dx) / 2 + (dy * dy) / 7 <= squared_radius) { 
                    fprintf(file_pointer, "PUSH 1\nPOP [%d]\n", element_coords);
                }
            }
        }
        
        fprintf(file_pointer, "DSPL\n");
        fprintf(file_pointer, "HLT\n");
        fclose(file_pointer);
    }
    
    else { PROCPRINTF("create_asm_for_circle: Файл не удалось открыть\n"); PROCGETCHAR(); }
}

stack_potential_error cpu_stack_dump(cpu_struct* cpu, stack_consts stack_mode) {
    if (stack_mode == CALL_STACK) {
        PROCPRINTF("осуществляю распечатку в режиме CALL_STACK\n");
        defended_stack* ptr_on_call_stack = cpu->call_stack;

        stack_potential_error stack_dump_processing = stack_dump(ptr_on_call_stack);

        return stack_dump_processing;
    }

    else if (stack_mode == CALC_STACK) {
        PROCPRINTF("осуществляю распечатку в режиме CALC_STACK\n");
        defended_stack* ptr_on_calc_stack = cpu->calculations_stack;

        stack_potential_error stack_dump_processing = stack_dump(ptr_on_calc_stack);

        return stack_dump_processing;
    }

    else { 
        PROCPRINTF("нераспознанный режим распечатки. у нас их всего два: CALL_STACK, CALC_STACK\n");

        return UNDEFINED_CPU_STACK_DUMP_MODE;
    }
}