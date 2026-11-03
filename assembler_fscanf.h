#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>


//   таблица хэшей строковых констант, которую я считаю один раз и сортирую по возрастанию (запаковать можно в int обычный)
//   вторая мысль: перед каждым свичем вызывать свою функцию count hash, которая будет считать хэш и не более от строки
//   потом я просто делаю бинарным поиском есть ли этот хэш в массиве посчитанных хэшей !!! 

#define MACROS_PROCESS_PUSH_POP_BY_HASH(cmd, function) \
    case cmd:                                          \
    {                                                  \ 
        potential_error pe = function;                 \ 
        if (pe != OK) { return pe; }                   \               
        break;                                         \
    } 


#define MACROS_PROCESS_ZERO_OPERAND_BY_HASH(cmd)                                   \
    case cmd:                                                                      \
    {                                                                              \ 
        potential_error pe = validate_zero_operands_commands(command_line_ptr,     \
                                                             cmd ## _ASM,          \
                                                             bytecode,             \
                                                             instruction_pointer); \ 
        if (pe != OK) { return pe; }                                               \               
        break;                                                                     \
    } 

#define MACROS_PROCESS_LABEL_COMMANDS_BY_HASH(cmd)                                \
    case cmd:                                                                     \
    {                                                                             \ 
        potential_error pe = validate_commands_only_labels(command_line_ptr,      \
                                                           labels_array,          \
                                                           cmd ## _ASM,           \
                                                           bytecode,              \
                                                           instruction_pointer);  \
        if (pe != OK) { return pe; }                                              \               
        break;                                                                    \
    } 

#define MACROS_DEFAULT_SITUATION()                                                \
    default:                                                                      \
        {                                                                         \
            potential_error pe = validate_labels_and_else(single_command_line,    \
                                                          labels_array,           \
                                                          *instruction_pointer);  \
            if (pe != OK) { return pe; }                                          \
            break;                                                                \
        }
        
#define MEMORY_BIT   (1u << 7)
#define REGISTER_BIT (1u << 6)
#define CONST_BIT    (1u << 5)
#define OPCODE_MASK  (0x1Fu)

#define MAX_LABELS_AMOUNT 128
#define MAX_LINE          256

#define ON_DEBUG

#ifdef  ON_DEBUG
    #define PRINTF(...) printf(__VA_ARGS__)
    #define GETCHAR()   getchar()
#endif

#ifndef ON_DEBUG
    #define PRINTF(...) (void(0))
    #define GETCHAR()   (void(0))
#endif

typedef double STACK_DATA_TYPE;

enum progon_index {
    FIRST  = 1,
    SECOND = 2,
};

enum potential_error  {
    OK                   = 0,
    
    WRITE_PROBLEMS       = 1,
    INVALID_FP           = 2,
    NO_ARG               = 3,
    NO_REGISTER          = 4,
    UNKNOWN_REG          = 5,
    FILE_NOT_OPENNED     = 6,
    INCORRECT_LABEL      = 7,
    UNKNOWN_COMMANDLETTE = 8,
    NO_LABEL_AFTER_CMD   = 9,
    COMPARISON_FAILED    = 10,
    STRING_OVERFLOW      = 11,
    INCORRECT_COMMAND_LINE = 12,
    EMPTY_STRING           = 13,
}; 

#include "generated_enums.h"

potential_error  obrabotka_faila(const char* filename_in, const  char* filename_out, const char* filename_out_after_first_asm);
potential_error  assemble_file_second(const char* filename_in, const char* filename_out, long long* labels_array, 
                                     progon_index progon_index, size_t* commands_and_arguments_counted);
potential_error  processing_labels(const char* command, long long* labels_array, long long instruction_pointer, FILE* out, progon_index progon_index);
potential_error  any_jump_processing(FILE* in, FILE* out, long long* labels_array, 
                                    const char* func_name, asm_commands_nums func_enum, progon_index progon_index);

potential_error  register_processing(FILE* in, FILE* out, const char* func_name, asm_commands_nums func_enum, progon_index progon_index);
potential_error  push_separate_validating(FILE* in, FILE* out, progon_index progon_index);

char*            initialize_file_buffer(const char* filename, struct stat* file_buffer_struct); 
potential_error  load_file_buffer(const char* filename, char* file_buffer, off_t struct_size);
                                    
uint64_t         pack_number(const char* string);

int              reg_code(const char* name);
void             clear_command_array(char* command);