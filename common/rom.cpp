#include <stdlib.h>
#include <string.h>

#include "rom.h"

int8_t                    g_aucLog2    [MAX_CU_SIZE + 1];

void initROM() {
    int c;

    // g_aucLog2[ x ]: log2(x), if x=1 -> 0, x=2 -> 1, x=4 -> 2, x=8 -> 3, x=16 -> 4, ...
    ::memset(g_aucLog2, 0, sizeof(g_aucLog2));
    c = 0;
    for( int i = 0, n = 0; i <= MAX_CU_SIZE; i++ ) {
        if( i == ( 1 << n ) ) {
            c = n;
            n++;
        }
        g_aucLog2    [i] = c;
    }
}
