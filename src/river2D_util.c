#include <stdint.h>
#include <string.h>

const char* river2D_contains
(
    const char *bigStr,
    const char *smallStr
){
    size_t subsize = strlen(smallStr);
    size_t size = strlen(bigStr) - subsize;

    for(size_t i = 0, j = 0; i < size; ++i)
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
            return &bigStr[i];
        }
retry:
        j = 0;
    }

    return 0;
}
