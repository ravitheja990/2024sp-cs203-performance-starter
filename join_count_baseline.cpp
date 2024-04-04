#include<cstdint>
#include "join_count.h"
#include <iostream>
#define START_C extern "C" { // this just hides the braces from the editor, so it won't try to indent everything.
#define END_C  }


START_C

extern uint64_t join_count(uint64_t *left_table, uint32_t left_table_size, uint64_t* right_table, uint32_t right_table_size)
{
    uint64_t r = 0;
 
    for(uint32_t i = 0; i < right_table_size; i++) {
        for(uint32_t j = 0; j < left_table_size; j++) {
            if (left_table[j] == right_table[i]) {
                r++;
            }
        }
    }
    return r;
}

END_C