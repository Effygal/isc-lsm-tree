#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int iad(int32_t max, int32_t n, int32_t *in, int32_t *out)
{
    int32_t *T = (int32_t*)calloc(1, sizeof(int32_t) * (uint64_t)max);
    int32_t t = 1;
    for (int i = 0; i < n; i++) {
        int32_t a = in[i];
        if (a >= max) {
            printf("ERROR: a[%d] = %d (max %d)\n", i, a, max);
            break;
        }
        if (T[a] == 0)
            out[i] = -1;
        else
            out[i] = t - T[a];
        T[a] = t;
        t++;
    }
    free(T);
    return 1;
}

int iad2(int32_t max, int32_t n, int32_t *in, int32_t *out,
         int32_t *t, int32_t *T)
{
    for (int i = 0; i < n; i++) {
        int32_t a = in[i];
        if (a >= max)
            return 0;
        if (T[a] == 0)
            out[i] = -1;
        else
            out[i] = *t - T[a];
        T[a] = *t;
        (*t)++;
    }
    return 1;
}



