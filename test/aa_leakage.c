#include "alloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef TEST_AA_LEAKAGE

#define AA_IMPLEMENTATION
#define AA_KEY char *
#define AA_VALUE size_t
#include "aa.h"

#if 0
#define log(fmt, ...) printf(fmt __VA_OPT__(, ) __VA_ARGS__)
#else
#define log(fmt, ...) (void)0
#endif

int main(void) {
    struct aa *a = aa_new();
    if (!a)
        return -1;

    for (size_t j = 0; j < 100; j++) {
        for (size_t i = 0; i < 100000; i++) {
            char key[32];
            snprintf(key, sizeof(key), "key_%zu", i);

            if (aa_set(a, key, i) != 0)
                assert(0);
        }

        log("After create:\n");
        for (size_t i = 0; i < 10000; i++) {
            char key[32];
            snprintf(key, sizeof(key), "key_%zu", i);

            log("%s\n", aa_get(a, key, NULL) ? "NULL" : key);
        }

        for (size_t i = 1000; i < 90000; i++) {
            char key[32];
            snprintf(key, sizeof(key), "key_%zu", i);

            aa_remove(a, key);
        }

        log("After remove:\n");
        for (size_t i = 0; i < 10000; i++) {
            char key[32];
            snprintf(key, sizeof(key), "key_%zu", i);

            log("%s\n", aa_get(a, key, NULL) ? "NULL" : key);
        }
    }

    aa_delete(a);

    log("Memory: %zu\n", __memory);
    assert(__memory == 0);

    return 0;
}

#endif /* TEST_AA_LEAKAGE */
