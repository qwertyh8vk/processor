#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "enum_djb.h"

#include "cmds_and_regs_manual.h"

const int max_len_for_space      = 4;
const int max_len_for_space_regs = 2;

int   enums_header_generator(const char* filename_out);
int   enums_hash_header_generator(const char* filename_out);
void  print_hash_values(FILE*     file_out,     const char* const ptr_on_array[], 
                        const int array_length, const int         space_aligment);