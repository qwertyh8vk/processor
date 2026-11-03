#include <ctype.h>
#include <stdint.h>

#define DJB2_ZERNO       5381
#define MAX_COMMAND_LEN    16

uint32_t djb2_calculate_enum_hash(const void *data);