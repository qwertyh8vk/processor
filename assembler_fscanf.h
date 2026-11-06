#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

//   таблица хэшей строковых констант, которую я считаю один раз и сортирую по возрастанию (запаковать можно в int обычный)
//   вторая мысль: перед каждым свичем вызывать свою функцию count hash, которая будет считать хэш и не более от строки
//   потом я просто делаю бинарным поиском есть ли этот хэш в массиве посчитанных хэшей !!! 

#define MACROS_PROCESS_PUSH_POP_BY_HASH(cmd)                                       \
    case cmd:                                                                      \
    {                                                                              \ 
        potential_error pe = validate_push_pop_command(command_line_ptr,           \
                                                       cmd ## _ASM,                \
                                                       bytecode,                   \
                                                       instruction_pointer,        \
                                                       bytecode_capacity);         \ 
        if (pe != OK) { return pe; }                                               \               
        break;                                                                     \
    } 

#define MACROS_PROCESS_ZERO_OPERAND_BY_HASH(cmd)                                   \
    case cmd:                                                                      \
    {                                                                              \ 
        potential_error pe = validate_zero_operands_commands(command_line_ptr,     \
                                                             cmd ## _ASM,          \
                                                             bytecode,             \
                                                             instruction_pointer,  \
                                                             bytecode_capacity);   \ 
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
                                                           instruction_pointer,   \
                                                           bytecode_capacity);    \
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

#define MEMORY_BIT   0b10000000
#define REGISTER_BIT 0b01000000
#define CONST_BIT    0b00100000

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

enum datatype {
    STACK_TYPE  = 1,
    LONG_LONG   = 2,
};

enum progon_index {
    FIRST  = 1,
    SECOND = 2,
};

enum potential_error  {
    OK                     = 0,
    
    WRITE_PROBLEMS         = 1,
    INVALID_FP             = 2,
    NO_ARG                 = 3,
    NO_REGISTER            = 4,
    UNKNOWN_REG            = 5,
    FILE_NOT_OPENNED       = 6,
    INCORRECT_LABEL        = 7,
    UNKNOWN_COMMANDLETTE   = 8,
    NO_LABEL_AFTER_CMD     = 9,
    COMPARISON_FAILED      = 10,
    STRING_OVERFLOW        = 11,
    INCORRECT_COMMAND_LINE = 12,
    EMPTY_STRING           = 13,
    INCORRECT_REG          = 14,
    BYTECODE_OVERFLOW      = 15,
    REALLOC_FAILED         = 16,
}; 

#include "generated_enums.h"

typedef struct {
    uint8_t                  flags;
    register_nums              reg;
    STACK_DATA_TYPE         number;

    //FIXME - Оч важная хуйня!!!!!
    long long                label;
} operand_data;

potential_error obrabotka_faila(char*      file_buffer,       uint8_t** bytecode, const char* exit_filename,
                                long long* bytecode_capacity, long long file_buffer_size);

char*           initialize_file_buffer(const char* filename, struct stat* file_buffer_struct); 

potential_error load_file_buffer(const char* filename, char* file_buffer, off_t struct_size);

potential_error assemble_file_second(char*         buffer_in,         uint8_t** bytecode, long long* labels_array, 
                                     progon_index  progon_index,      size_t*   commands_and_arguments_counted, 
                                     long long*    bytecode_capacity, long long file_buffer_size);

potential_error process_command_byhash(uint32_t       command_hash,         char*           single_command_line, 
                                        char*         command_line_ptr,     long long*      labels_array,
                                        long long*    instruction_pointer,  uint8_t**       bytecode, 
                                        long long*    bytecode_capacity);

potential_error write_operand(asm_commands_nums     func_asm_code,
                              uint8_t                       flags,
                              register_nums                   reg,
                              void*                        chislo,
                              uint8_t**                  bytecode,
                              long long*      instruction_pointer,
                              long long*        bytecode_capacity,
                              datatype      what_we_are_memcpying);

potential_error parse_operand_push_pop(char*               command_line_ptr, 
                                       operand_data*  ptr_on_operand_struct, 
                                       int*                     debug_index,
                                       asm_commands_nums      func_asm_code);   

potential_error validate_push_pop_command(char*              command_line_ptr, 
                                          asm_commands_nums  func_asm_code, 
                                          uint8_t**          bytecode,
                                          long long*         instruction_pointer,
                                          long long*         bytecode_capacity);

register_nums   determine_register(char* reg);

potential_error validate_zero_operands_commands(char*              command_line_ptr, 
                                                asm_commands_nums  func_asm_code, 
                                                uint8_t**          bytecode,
                                                long long*         instruction_pointer,
                                                long long*         bytecode_capacity); 

potential_error check_string_after_arg(char* command_line_ptr); 

potential_error validate_commands_only_labels(char*              command_line_ptr, 
                                              long long*         labels_array, 
                                              asm_commands_nums  func_asm_code,
                                              uint8_t**          bytecode,
                                              long long*         instruction_pointer,
                                              long long*         bytecode_capacity);

potential_error validate_labels_and_else(char*       single_command_line, 
                                         long long*  labels_array, 
                                         long long   current_instruction);   

potential_error recalloc(uint8_t** bytecode, long long* bytecode_capacity);

potential_error validate_string_length(char* buffer_in, int debug_index);

potential_error write_bytecode_in_binary(const char* exit_filename, uint8_t* bytecode, long long valuable_bytes);

uint32_t        pull_the_fucking_command(const char* single_command_line, int* command_length);

void            clear_command_array(char* command);

void            debugger_like_printf(char* file_buffer, struct stat* to_be_assemblered_struct);

void            zero_after_calloc(uint8_t* bytecode, long long start_ptr, long long end_ptr);