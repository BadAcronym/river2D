#include <stdint.h>
#include <string.h>

char* river2D_contains
(
    const char *bigStr,
    const char *smallStr
){
    char *cptr = 0;
    uint32_t subsize = strlen(smallStr);
    uint32_t size = strlen(bigStr) - subsize;

    for(uint32_t i = 0, j = 0; i < size; ++i)
    {
        if(bigStr[i] == smallStr[j])
        {
            for(; j < subsize; ++j)
            {
                if(bigStr[i + j] != smallStr[j])
                {
                    goto retry;
                }
            }
            return cptr;
        }
retry:
        j = 0;
    }

    return cptr;
}
