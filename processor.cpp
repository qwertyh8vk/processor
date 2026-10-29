#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include "stack_v3.h"

#define REGS_AMOUNT       4
#define BASIC_LENGTH      2 
#define FUNCS_AMOUNT     23
#define MAX_AMT_OF_DGTS 150 

#define JMP_MACROS_INITIALIZE(func_name, mode_1, mode_2)   \
    cpu_status func_name##_asm(cpu_struct* cpu) { \                  
        return jmp_cond_universal(cpu, mode_1, mode_2);    \
    }                                                                         

enum cmp_compare_res {
    BELOW = -1,
    EQUAL =  0,
    ABOVE =  1,
};

enum stack_consts {
    CALL_STACK = 1,
    CALC_STACK = 2,
};

enum cpu_status {
    CPU_FINE           =  0,
    CPU_HLT            =  4,
    CPU_ERR            = -1,
    CPU_UNK_CMD        = -2,
    CPU_UNCHECKED_CASE = -3,
};

enum cpu_side_errors {
    
};

typedef struct        cpu_struct cpu_struct;
typedef               cpu_status (*cmd_handler)(cpu_struct* cpu);

cpu_struct*           initialize_cpu_struct(const char* filename, size_t amounts_of_cmds_and_args);
size_t                get_file_cmds_n_args_amount(const char* filename);
stack_potential_error cpu_stack_dump(cpu_struct* cpu, stack_consts stack_mode);
void                  cpu_code_array_dump(cpu_struct* cpu, size_t size_of_array);

cpu_status            jmp_cond_universal(cpu_struct* cpu, 
                                         cmp_compare_res mode_1, cmp_compare_res mode_2);

cpu_status            push_asm(cpu_struct*  cpu);
cpu_status            pushr_asm(cpu_struct* cpu);
cpu_status            pop_asm(cpu_struct*   cpu); 
cpu_status            popr_asm(cpu_struct*  cpu); 

cpu_status            add_asm(cpu_struct*   cpu); 
cpu_status            sub_asm(cpu_struct*   cpu); 
cpu_status            mul_asm(cpu_struct*   cpu); 
cpu_status            div_asm(cpu_struct*   cpu); 
cpu_status            sqrt_asm(cpu_struct*  cpu); 
cpu_status            mod_asm(cpu_struct*   cpu); 

cpu_status            jmp_asm(cpu_struct*   cpu); 
cpu_status            jmpa_asm(cpu_struct*  cpu); 
cpu_status            jmpb_asm(cpu_struct*  cpu); 
cpu_status            jae_asm(cpu_struct*   cpu); 
cpu_status            jbe_asm(cpu_struct*   cpu); 
cpu_status            jmpe_asm(cpu_struct*  cpu); 

cpu_status            call_asm(cpu_struct*  cpu); 
cpu_status            ret_asm(cpu_struct*   cpu); 

cpu_status            in_asm(cpu_struct*    cpu); 
cpu_status            out_asm(cpu_struct*   cpu); 

cpu_status            hlt_asm(cpu_struct*   cpu); 

cpu_status            dup_asm(cpu_struct*   cpu);
cpu_status            cmp_asm(cpu_struct*   cpu);

cpu_status            cpu_work_cycle(cpu_struct* cpu);
cpu_status            finish_instruction(cpu_struct* cpu, stack_potential_error stack_opname_status, unsigned int jumps);

cmd_handler asm_funcs[FUNCS_AMOUNT] {
    /* первый логический блок */
    push_asm,
    pushr_asm,
    pop_asm,
    popr_asm,

    /* второй логический блок */
    add_asm,
    sub_asm,
    mul_asm,
    div_asm,
    sqrt_asm,
    mod_asm,

    /* третий логический блок */
    jmp_asm,
    jmpa_asm,
    jmpb_asm,
    jae_asm,
    jbe_asm,
    jmpe_asm,

    /* четвертый логический блок */
    call_asm,
    ret_asm,

    /* пятый логический блок */
    in_asm,
    out_asm,
    
    /* шестой логический блок */
    hlt_asm,

    /* всё что не вошло в предыдущие блоки*/
    dup_asm,
    cmp_asm,
};

int main() {
    const char* assembler_to_cpu = "assembler_res.txt";
    size_t cmds_n_args_amount = get_file_cmds_n_args_amount(assembler_to_cpu);

    PRINTF("Что насчитала моя функция get_file_cmds_n_args_amount: [%zu]\n", cmds_n_args_amount);
    GETCHAR();

    cpu_struct* ryzen7_5700 = initialize_cpu_struct(assembler_to_cpu, cmds_n_args_amount);

    cpu_stack_dump(ryzen7_5700, CALL_STACK);
    cpu_stack_dump(ryzen7_5700, CALC_STACK);

    cpu_work_cycle(ryzen7_5700);

    return OK;
}

#include "stack_v3_ot.h"

struct cpu_struct {
    STACK_DATA_TYPE* code;

    long long        instruction_pointer;
    size_t           code_obsolet_size;

    defended_stack*  calculations_stack;
    defended_stack*  call_stack;

    STACK_DATA_TYPE  regs[REGS_AMOUNT];
    cpu_status       processor_status;

    cmp_compare_res  cmp_flag;
};

cpu_status push_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_we_would_push = (cpu->code)[cpu->instruction_pointer + 1];

    PRINTF("Без отладки никуда. что мы считали: [%lf]\n", value_we_would_push);

    cpu_code_array_dump(cpu, cpu->code_obsolet_size);
    defended_stack* ptr_on_cpu_calc_stack   = cpu->calculations_stack;
    stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, value_we_would_push);

    return finish_instruction(cpu, stack_push_status, 2);
}

cpu_status pushr_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE index_of_cpu_regs_arr = (cpu->code)[cpu->instruction_pointer + 1];
    PRINTF("Индекс массива REGS который мы прочитали: [%d]\n", (int)index_of_cpu_regs_arr); // !!

    assert(index_of_cpu_regs_arr >= 0 && index_of_cpu_regs_arr <= 3);

    STACK_DATA_TYPE value_we_would_push = cpu->regs[(int)index_of_cpu_regs_arr];
    PRINTF("Значение регистра по указанному индексу: [%lf]\n", value_we_would_push);

    cpu_code_array_dump(cpu, cpu->code_obsolet_size);
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;
    stack_potential_error stack_pushr_status = stack_push(ptr_on_cpu_calc_stack, value_we_would_push);

    return finish_instruction(cpu, stack_pushr_status, 2);
}

cpu_status pop_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, NULL); // никуда не сохраняем

    return finish_instruction(cpu, stack_pop_status, 1);
}

cpu_status popr_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE index_of_cpu_regs_arr = (cpu->code)[cpu->instruction_pointer + 1];
    PRINTF("Индекс массива REGS который мы прочитали: [%d]\n", (int)index_of_cpu_regs_arr);
    assert(index_of_cpu_regs_arr >= 0 && index_of_cpu_regs_arr <= 3);

    STACK_DATA_TYPE value_we_would_push = 0;

    cpu_code_array_dump(cpu, cpu->code_obsolet_size);
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;
    stack_potential_error stack_popr_status = stack_pop(ptr_on_cpu_calc_stack, &value_we_would_push);

    if (stack_popr_status == OK) {
        cpu->regs[(int)index_of_cpu_regs_arr] = value_we_would_push;
        PRINTF("Значение регистра по указанному индексу: [%lf]\n", value_we_would_push);

        stack_dump(ptr_on_cpu_calc_stack);

        cpu->instruction_pointer += 2; // потому что мы исполнили push и считали аргумент push'a
        cpu_code_array_dump(cpu, cpu->code_obsolet_size);

        return CPU_FINE;
    }

    else { return CPU_ERR; }
}

// ----------------------------------------------------------------------------------------------------------------

cpu_status add_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

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

cpu_status sub_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;
    STACK_DATA_TYPE value_b = 0;

    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;
    
    stack_potential_error stack_pop_a_status = stack_pop(ptr_on_cpu_calc_stack, &value_a);
    stack_potential_error stack_pop_b_status = stack_pop(ptr_on_cpu_calc_stack, &value_b);

    if ((stack_pop_a_status == OK) && (stack_pop_b_status == OK)) {
        STACK_DATA_TYPE diff_betw_a_b = value_a - value_b;

        stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, diff_betw_a_b);

        return finish_instruction(cpu, stack_push_status, 1);
    }

    else { return CPU_ERR; }
}

cpu_status mul_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;
    STACK_DATA_TYPE value_b = 0;

    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    stack_potential_error stack_pop_a_status = stack_pop(ptr_on_cpu_calc_stack, &value_a);
    stack_potential_error stack_pop_b_status = stack_pop(ptr_on_cpu_calc_stack, &value_b);

    if ((stack_pop_a_status == OK) && (stack_pop_b_status == OK)) {
        STACK_DATA_TYPE mul_of_a_b = value_a * value_b;

        stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, mul_of_a_b); 

        return finish_instruction(cpu, stack_push_status, 1);
    }
    
    else { return CPU_ERR; }
}

cpu_status div_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;
    STACK_DATA_TYPE value_b = 0;

    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    stack_potential_error stack_pop_a_status = stack_pop(ptr_on_cpu_calc_stack, &value_a);
    stack_potential_error stack_pop_b_status = stack_pop(ptr_on_cpu_calc_stack, &value_b);

    if ((stack_pop_a_status == OK) && (stack_pop_b_status == OK)) {
        STACK_DATA_TYPE mul_of_a_b = 0;

        if (value_b != 0) { mul_of_a_b = value_a / value_b; }

        else {
            PRINTF("Деление на ноль я пока ещё не определил. мб выкачу в дальнейших обновлениях\n");
            GETCHAR();

            return CPU_ERR;
        }

        stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, mul_of_a_b);

        return finish_instruction(cpu, stack_push_status, 1);
    }

    else { return CPU_ERR; }
}

cpu_status sqrt_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_a = 0;

    defended_stack*  ptr_on_cpu_calc_stack = cpu->calculations_stack;
    stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, &value_a);

    if (stack_pop_status == OK) {
        if (value_a >= 0) {
            STACK_DATA_TYPE sqrt_a = sqrt(value_a); 

            stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, sqrt_a);

            return finish_instruction(cpu, stack_push_status, 1);
        }

        else {
            PRINTF("Корень квадратный из отр. числа пока не определил. мб выкачу в дальнейших обновлениях\n");
            GETCHAR();

            return CPU_ERR;
        }
    }

    else { return CPU_ERR;  }
}

cpu_status mod_asm(cpu_struct* cpu) {
    return CPU_FINE; // пустая функция пока что
}

// ----------------------------------------------------------------------------------------------------------------

cpu_status jmp_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    long long where_we_want_to_jump = (cpu->code)[cpu->instruction_pointer + 1];
    const size_t code_obsolet_size  = cpu->code_obsolet_size;

    PRINTF("куда мы хотим прыгнуть (на какой индекс): [%lld]\n", where_we_want_to_jump);
    GETCHAR();

    if (where_we_want_to_jump >= 0 && where_we_want_to_jump < code_obsolet_size) { cpu->instruction_pointer = where_we_want_to_jump; }
    
    else {
        PRINTF("Нубля сюда прыгать я ещё не научился\n");
        GETCHAR();

        return CPU_ERR;
    }

    PRINTF("Установил точку откуда будем выполнять команды\n");
    cpu_code_array_dump(cpu, code_obsolet_size);

    return CPU_FINE;
}

cpu_status jmp_cond_universal(cpu_struct* cpu, cmp_compare_res mode_1, cmp_compare_res mode_2) {
    PRINTF(PURPLE "ВЫЗВАННЫЙ РЕЖИМ ИМЕЕТ КОД [%d] %d]\n" RESET, mode_1, mode_2); // можно переделать в таблицу кодов , для этого нужно завести структуру

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

    GETCHAR();

    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    long long where_we_want_to_jump = (cpu->code)[cpu->instruction_pointer + 1];
    const size_t code_obsolet_size  = cpu->code_obsolet_size;

    PRINTF("куда мы хотим прыгнуть (на какой индекс): [%lld]\n", where_we_want_to_jump);
    GETCHAR();

    int cpu_current_flag = cpu->cmp_flag;

    if (cpu_current_flag == mode_1 || cpu_current_flag == mode_2) {
        if (where_we_want_to_jump >= 0 && where_we_want_to_jump < code_obsolet_size) { 
            cpu->instruction_pointer = where_we_want_to_jump; 
            PRINTF("Установил точку откуда будем выполнять команды\n");
            cpu_code_array_dump(cpu, code_obsolet_size);

            return CPU_FINE;
        }

        else {
            PRINTF("Нубля сюда прыгать я ещё не научился\n");
            GETCHAR();

            return CPU_ERR;
        }
    } // one more КОПИПАСТ, который можно починить

    else {
        PRINTF("Ластовый вызванный CMP имеет значение не равное выбранному режиму: cmp_flag value = [%d]\n", cpu_current_flag);
        GETCHAR();
        
        return CPU_FINE;
    }
}

JMP_MACROS_INITIALIZE(jmpa, ABOVE, ABOVE);
JMP_MACROS_INITIALIZE(jmpb, BELOW, BELOW);
JMP_MACROS_INITIALIZE(jae,  EQUAL, ABOVE);
JMP_MACROS_INITIALIZE(jbe,  EQUAL, BELOW);
JMP_MACROS_INITIALIZE(jmpe, EQUAL, EQUAL);

// ----------------------------------------------------------------------------------------------------------------

cpu_status call_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    defended_stack* ptr_on_cpu_call_stack       = cpu->call_stack;

    long long where_we_want_to_start_from_again = (cpu->code)[cpu->instruction_pointer + 2]; // + 2 потому что + 1 это прыжок на аргумент call.
    long long where_we_want_to_jump             = (cpu->code)[cpu->instruction_pointer + 1];

    const size_t code_obsolet_size              = cpu->code_obsolet_size;

    PRINTF("Значение, которое возьмёт ret в качестве ориентира куда возвращаться: [%lld]\n", where_we_want_to_start_from_again);
    GETCHAR();

    stack_potential_error stack_push_wwwtsf_status = BAD_DEFAULT;

    if (where_we_want_to_start_from_again < code_obsolet_size) {
        stack_push_wwwtsf_status = stack_push(ptr_on_cpu_call_stack, where_we_want_to_start_from_again);
        stack_dump(ptr_on_cpu_call_stack);
    }

    else {
        PRINTF(DARK_PURPLE "Походу после CALL ничего не идёт, в таком случае ничего не делаю " RED "(Относим это к ошибке)\n" RESET);
        GETCHAR();

        return CPU_ERR;
    }
    
    if (stack_push_wwwtsf_status == OK) {
        if (where_we_want_to_jump >= 0 && where_we_want_to_jump < code_obsolet_size) { 
            cpu_code_array_dump(cpu, code_obsolet_size);

            cpu->instruction_pointer = where_we_want_to_jump; 
            PRINTF("Установил точку откуда будем выполнять команды\n");

            cpu_code_array_dump(cpu, code_obsolet_size);

            return CPU_FINE;
        }

        else {
            PRINTF("Нубля сюда прыгать я ещё не научился\n");
            GETCHAR();

            return CPU_ERR;
        }
    }

    else { return CPU_ERR; }
}

cpu_status ret_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    defended_stack* ptr_on_cpu_call_stack = cpu->call_stack;
    STACK_DATA_TYPE where_we_want_to_start_from_again = 0;

    stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_call_stack, &where_we_want_to_start_from_again);
    if (stack_pop_status == OK) {
        stack_dump(ptr_on_cpu_call_stack);

        PRINTF("Индекс который сейчас вытащил ret: [%lld]\n", (long long)where_we_want_to_start_from_again);

        cpu_code_array_dump(cpu, cpu->code_obsolet_size);

        cpu->instruction_pointer = (long long)where_we_want_to_start_from_again;
        PRINTF("Установил нужный индекс в ret'e\n");
        GETCHAR();

        cpu_code_array_dump(cpu, cpu->code_obsolet_size);

        return CPU_FINE;
    }

    else { return CPU_ERR; }
}

// ----------------------------------------------------------------------------------------------------------------

cpu_status in_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);
    PRINTF("НУ что, товарищ пользователь, введите число которое я запушу в calc_stack: ");
    char  read_string_typed_by_user[MAX_AMT_OF_DGTS] = {};
    char* fgets_result = fgets(read_string_typed_by_user, MAX_AMT_OF_DGTS, stdin);

    if (fgets_result != NULL) {
        PRINTF("успешно записана строка в массив read_string_typed_by_user\n");
        GETCHAR();

        defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

        char* end_ptr = NULL;
        STACK_DATA_TYPE value_to_push_from_stdin = strtod(read_string_typed_by_user, &end_ptr);
        PRINTF("На каком символе закончилось преобразование strtod: [%s]\n", end_ptr);
        GETCHAR();

        stack_potential_error stack_push_status = stack_push(ptr_on_cpu_calc_stack, value_to_push_from_stdin);

        return finish_instruction(cpu, stack_push_status, 1);
    }

    else {
        PRINTF("fgets_result вернул NULL, нихуя себе!\n");
        GETCHAR();

        return CPU_ERR;
    }
}

cpu_status out_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_to_out = 0;
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    cpu_status stack_dup_status = dup_asm(cpu);

    if (stack_dup_status == CPU_FINE) {
        PRINTF(GREEN "Успешно " RESET "сдублировал число\n");
        GETCHAR();

        stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, &value_to_out);
        if (stack_pop_status == OK) {
            fprintf(stdout, "ласт число из стэка: " PURPLE "[%lf]\n" RESET, value_to_out);

            return CPU_FINE;
        }

        else { return CPU_ERR; }
    }

    else { return CPU_ERR; }
}

cpu_status hlt_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    return CPU_HLT;
}

// ----------------------------------------------------------------------------------------------------------------

cpu_status dup_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    STACK_DATA_TYPE value_to_dup = 0;
    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    stack_potential_error stack_pop_status = stack_pop(ptr_on_cpu_calc_stack, &value_to_dup);

    if (stack_pop_status == OK) {
        PRINTF("Что я сохранил в value_to_dup из стэка: [%lf]\n", value_to_dup);
        GETCHAR();
        
        stack_potential_error stack_push_first_status  = stack_push(ptr_on_cpu_calc_stack, value_to_dup);
        stack_potential_error stack_push_second_status = stack_push(ptr_on_cpu_calc_stack, value_to_dup);
        cpu_status err_code = finish_instruction(cpu, stack_push_first_status, 0); 

        if (err_code != CPU_FINE) { return err_code; }
        else                      { return finish_instruction(cpu, stack_push_second_status, 1); }

    } // по-хорошему я должон обрабатывать значения возвращаемые всеми этими функциями god damn!

    else { return CPU_ERR; }
}

cpu_status cmp_asm(cpu_struct* cpu) {
    PRINTF("Текущий индекс команды: [%lld]\n", cpu->instruction_pointer);

    defended_stack* ptr_on_cpu_calc_stack = cpu->calculations_stack;

    STACK_DATA_TYPE value_a_to_cmp_from_stack = 0;
    STACK_DATA_TYPE value_b_to_cmp_from_stack = 0; // собственно пока CMP имеет именно такой смысл!

    stack_potential_error stack_pop_status_a = stack_pop(ptr_on_cpu_calc_stack, &value_a_to_cmp_from_stack);
    stack_potential_error stack_pop_status_b = stack_pop(ptr_on_cpu_calc_stack, &value_b_to_cmp_from_stack);

    if (stack_pop_status_a == OK && stack_pop_status_b == OK) {
        stack_dump(ptr_on_cpu_calc_stack);

        PRINTF(YELLOW "Вот что я сохранил в value_a: [%lf]\n", value_a_to_cmp_from_stack);
        PRINTF("Вот что я сохранил в value_b: [%lf]\n" RESET,  value_b_to_cmp_from_stack);
        GETCHAR();
        
        if      (value_b_to_cmp_from_stack < value_a_to_cmp_from_stack) { cpu->cmp_flag = BELOW; }
        else if (value_b_to_cmp_from_stack > value_a_to_cmp_from_stack) { cpu->cmp_flag = ABOVE; }
        else                                                            { cpu->cmp_flag = EQUAL; }

        stack_potential_error stack_push_b_status = stack_push(ptr_on_cpu_calc_stack, value_b_to_cmp_from_stack);
        stack_potential_error stack_push_a_status = stack_push(ptr_on_cpu_calc_stack, value_a_to_cmp_from_stack);

        if (stack_push_b_status == OK && stack_push_a_status == OK) { 
            stack_dump(ptr_on_cpu_calc_stack);
            PRINTF("Запушил два числа с сохранением порядка\n");
            GETCHAR();

            return CPU_FINE;
        }

        else { return CPU_ERR; }
    }

    else { return CPU_ERR; }
}

// ----------------------------------------------------------------------------------------------------------------

cpu_status finish_instruction(cpu_struct* cpu, stack_potential_error stack_opname_status, unsigned int jumps) {
    if (stack_opname_status == OK) {
        cpu->instruction_pointer += jumps; 
        cpu_code_array_dump(cpu, cpu->code_obsolet_size);

        return CPU_FINE;
    }

    else { return CPU_ERR; }
}

cpu_struct* initialize_cpu_struct(const char* filename, size_t amounts_of_cmds_and_args) {
    assert(filename);

    FILE* file_pointer = fopen(filename, "r");

    if (file_pointer != NULL) {
        cpu_struct* ryzen7_5700 = (cpu_struct *)calloc(1, sizeof(cpu_struct)); // создали структурку

        ryzen7_5700->code                      = (STACK_DATA_TYPE *)calloc(amounts_of_cmds_and_args, sizeof(STACK_DATA_TYPE));

        size_t counter_of_cmds_and_args_safety = 0;
        STACK_DATA_TYPE value_read_by_fscanf   = 0;

        // пропускаем служебную строку "Service information for CPU: [n]"
        fscanf(file_pointer, "%*[^\n]");

        int i = 0;

        PRINTF(PURPLE "==============================================================================================================\n" RESET);

        while((fscanf(file_pointer, "%lf", &value_read_by_fscanf) == 1) && i < amounts_of_cmds_and_args) {
            PRINTF(PURPLE "Успешно прочитана команда/аргумент команды\n" RESET);
            GETCHAR();

            counter_of_cmds_and_args_safety += 1; // прибавили команду || аргумент который прочитали
            (ryzen7_5700->code)[i] = value_read_by_fscanf;
            i++;
        }

        PRINTF(PURPLE "==============================================================================================================\n" RESET);

        if (amounts_of_cmds_and_args == counter_of_cmds_and_args_safety) {
            ryzen7_5700->instruction_pointer       = 0; // по умолчанию указываем на нулевую (первую) строчку
            ryzen7_5700->code_obsolet_size         = amounts_of_cmds_and_args;
            ryzen7_5700->calculations_stack        = stack_init(BASIC_LENGTH);
            ryzen7_5700->call_stack                = stack_init(BASIC_LENGTH);
            ryzen7_5700->processor_status          = CPU_FINE;
            ryzen7_5700->cmp_flag                  = EQUAL; // 0 по дефолту, что достаточно логично
            
            for (int i = 0; i < REGS_AMOUNT; i++) {
                (ryzen7_5700->regs)[i] = 0; // значения всех регистров по умолчанию выставляем в ноль.
            }

            return ryzen7_5700;
        }

        else {
            PRINTF("говорит функция initialize_cpu_struct!\nМы блять недосчитались команд или аргументов!\n");

            return NULL;
        }
    }

    else {
        PRINTF("говорит функция initialize_cpu_struct!\nфайл блять не открылся!\n");

        return NULL;
    }
}

size_t get_file_cmds_n_args_amount(const char* filename) {
    assert(filename);
    
    FILE* file = fopen(filename, "r");

    if (file != NULL) {
        size_t cmds_n_args_amount  = 0;

        int amount_of_read_nums = fscanf(file, "%*[^[][%zu]", &cmds_n_args_amount);
        fclose(file);

        if (amount_of_read_nums == 1) {
            PRINTF("число считалось: [%zu]\n", cmds_n_args_amount);

            return cmds_n_args_amount;
        }

        else {
            PRINTF("\nГоворит функция get_file_cmds_n_args_amount\nЧисло не считалось!!! работайте с типами.\n");
    
            return 0;
        }
    }

    PRINTF("Говорит функция get_file_cmds_n_args_amount\nФайл не был открыт.\n");

    return 0;
}

cpu_status cpu_work_cycle(cpu_struct* cpu) {
    long long instruction_pointer  = cpu->instruction_pointer;
    const size_t code_obsolet_size = cpu->code_obsolet_size;

    /* условия выхода: адекватное - мы дошли до HLT, или мы дошли до конца кода, но при этом HLT не встретили - в таком случае мы шлём пользователя нахуй и дропаем программу!*/
    while(instruction_pointer < code_obsolet_size) {
        /* пока работаем - исполняй команды. обрабатывай статус после каждой команды */
        
        int what_we_are_executing = ((cpu->code)[cpu->instruction_pointer] - 1);

        if (what_we_are_executing >= 0 && what_we_are_executing <= FUNCS_AMOUNT - 1) {
            cpu_status renew_status_after_command = asm_funcs[what_we_are_executing](cpu);

            if (renew_status_after_command == CPU_HLT) {
                PRINTF(GREEN "Мы дошли до хальт. Ниче стращного щас просто ебнем програмку и всё.\n" RESET);

                cpu->processor_status = CPU_HLT; // в структурку всё таки это надо записать
                return CPU_HLT;
            }

            else if (renew_status_after_command == CPU_ERR) {
                PRINTF("ПРОИЗОШЛА ОШИБКА! ПИЗДЕЦ! Да шучу я. всё фиксится. Код функции, после которой произошел бум: [%lld]\n", cpu->instruction_pointer);

                cpu->processor_status = CPU_ERR;
                return CPU_ERR;
            }
        }

        else {
            PRINTF("Тут какие варианты. 99 процентов что цикл работает неправильно, и 1 процент что мы реально наткнулись на неизвестную команду\n");

            cpu->processor_status = CPU_UNK_CMD;
            return CPU_UNK_CMD;
        }
    }

    PRINTF("До сюда мы не должны были дойти, поэтому это необработанный случай\n");
    cpu->processor_status = CPU_UNCHECKED_CASE;

    return CPU_UNCHECKED_CASE;
}

void cpu_code_array_dump(cpu_struct* cpu, size_t size_of_array) {
    assert(cpu);

    printf(DARK_PURPLE "==============================================================================================================\n");

    PRINTF("CPU CURR INST: [%llu]\n", cpu->instruction_pointer);

    for (int i = 0; i < size_of_array; i++) {
        if (i == cpu->instruction_pointer) {
            printf(PURPLE "[%lf]\n" RESET, (cpu->code)[i]);
        }

        else {
            printf(DARK_PURPLE "[%lf]\n", (cpu->code)[i]);
        }
    }

    printf("\n");
    printf(    "==============================================================================================================\n" RESET);
}

stack_potential_error cpu_stack_dump(cpu_struct* cpu, stack_consts stack_mode) {
    if (stack_mode == CALL_STACK) {
        PRINTF("осуществляю распечатку в режиме CALL_STACK\n");
        defended_stack* ptr_on_call_stack = cpu->call_stack;

        stack_potential_error stack_dump_processing = stack_dump(ptr_on_call_stack);

        return stack_dump_processing;
    }

    else if (stack_mode == CALC_STACK) {
        PRINTF("осуществляю распечатку в режиме CALC_STACK\n");
        defended_stack* ptr_on_calc_stack = cpu->calculations_stack;

        stack_potential_error stack_dump_processing = stack_dump(ptr_on_calc_stack);

        return stack_dump_processing;
    }

    else { 
        PRINTF("нераспознанный режим распечатки. у нас их всего два: CALL_STACK, CALC_STACK\n");

        return UNDEFINED_CPU_STACK_DUMP_MODE;
    }
}