#include "alloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef TEST_AA_INT

#define AA_KEY int
#define AA_VALUE int
#define AA_IMPLEMENTATION
#include "aa.h"

int main(void) {
    printf("Allocator test\n");
    int *memory = (int *)__malloc(sizeof(int) * 1024);

    printf("Heap after allocation: %zu\n", __memory);
    printf("Pointer length: %zu\n", __len(memory));

    assert(__memory == __len(memory));

    __free(memory);
    printf("Heap after deallocation: %lu\n", __memory);

    struct aa *a = aa_new();
    assert(a != NULL);

    for (int i = 0; i < 1'000'000; i++) aa_set(a, i, i + 1);
    printf("Heap of a[%zu]: %lu\n", a->used, __memory);

    for (int i = 2000; i < 1'000'000; i++) aa_remove(a, i);
    aa_rehash(a);

    printf("Heap of a[%zu]: %lu\n", a->used, __memory);
    printf("a[1000]: %d\n", aa_get(a, 1000));

    aa_free(a);

    assert(__memory == 0);

    return 0;
}

#endif /* TEST_AA_INT */
