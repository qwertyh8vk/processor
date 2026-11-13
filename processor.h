#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#include "stack_v3.h"
#include "generated_enums.h"

#define RAM_MB                3025
#define ARR_WIDTH               55
#define ARR_HEIGHT              55

#define MEMORY_BIT      0b10000000
#define REGISTER_BIT    0b01000000
#define CONST_BIT       0b00100000

#define OPCODE_MASKA    0b00011111
#define MODE_MASKA      0b11100000

#define ONLY_RAM        0b10000000
#define REG_CONST       0b01100000
#define CONST           0b00100000
#define NOTHING         0b00000000

// #define PROCDEBUG

#ifdef  PROCDEBUG
    #define PROCPRINTF(...)  printf(__VA_ARGS__)
    #define PROCGETCHAR(...) getchar(__VA_ARGS__)
#endif
#ifndef PROCDEBUG
    #define PROCPRINTF(...)  (void(0))
    #define PROCGETCHAR(...) (void(0))
#endif

#define JMP_MACROS_INITIALIZE(func_name, mode_1, mode_2)   \
    cpu_status func_name##_asm(cpu_struct* cpu) {          \
        return jmp_cond_universal(cpu, mode_1, mode_2);    \
    }                                                                         

enum basic_but_important_consts {
    MAX_AMT_OF_DGTS = 150,
    BASIC_LENGTH    = 2,
};

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
    CPU_FINE                   =  0,

    CPU_HLT                    =  4,

    CPU_ERR                    = -1,
    CPU_UNK_CMD                = -2,
    CPU_UNCHECKED_CASE         = -3,
    NUMBER_IS_NOT_WHORE        = -4,
    INVALID_VIDEORAM_INDEX     = -5,
    BAD_COMMAND_MODE           = -6,
    CPU_DESTROY_USER_SYSTEM    = -7,
};

#include "cmds_and_regs_manual.h"

typedef struct        cpu_struct cpu_struct;
typedef               cpu_status (*cmd_handler)(cpu_struct* cpu);

cpu_struct*           initialize_cpu_struct(const char* filename, size_t bytes_read_from_bin);
size_t                get_bytes_in_bin_amount(const char* filename);
stack_potential_error cpu_stack_dump(cpu_struct* cpu, stack_consts stack_mode);
void                  cpu_code_array_dump(cpu_struct* cpu, size_t size_of_array);
void                  create_asm_for_circle(const char* filename, int circle_radius);
cpu_status            validate_videoram_index(STACK_DATA_TYPE number);
cpu_status            check_whole_number(STACK_DATA_TYPE number);

cpu_status            jmp_cond_universal(cpu_struct* cpu, 
                                         cmp_compare_res mode_1, 
                                         cmp_compare_res mode_2);

cpu_status            finish_instruction(cpu_struct*           cpu, 
                                         stack_potential_error stack_opname_status, 
                                         long long             bytes_amount);

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
cpu_status            over_asm(cpu_struct*  cpu);
cpu_status            swap_asm(cpu_struct*  cpu);
cpu_status            dspl_asm(cpu_struct*  cpu);

cpu_status            cpu_work_cycle(cpu_struct* cpu);


cmd_handler asm_funcs[N_CMDS] {
    /* первый логический блок */
    push_asm,
    pop_asm,

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
    over_asm,
    swap_asm,
    dspl_asm,
};