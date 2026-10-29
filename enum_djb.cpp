#include "enum_djb.h"

uint32_t djb2_calculate_enum_hash(const void *data) {
    const unsigned char* pointer_on_data = (const unsigned char *)data;
    uint32_t hash = DJB2_ZERNO;

    for (int i = 0; (*pointer_on_data != '\0' && i < MAX_COMMAND_LEN); pointer_on_data++, i++)
        hash = ((hash << 5) + hash) + *pointer_on_data;

    return hash;
}