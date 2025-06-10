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
    printf("Pointer length: %zu\n", __len(memory) / sizeof(int));

    assert(__memory == __len(memory));

    __free(memory);
    printf("Heap after deallocation: %lu\n", __memory);

    struct aa *a = aa_new();
    assert(a != NULL);

    for (int i = 0; i < 1000000; i++) /* \n */
        assert(aa_set(a, i, i + 1) == 0);
    printf("Heap of a[%zu]: %lu\n", a->used, __memory);

    for (int i = 2000; i < 1000000; i++) /* \n */
        assert(aa_remove(a, i) == true);
    aa_rehash(a);

    AA_VALUE value;
    printf("Heap of a[%zu]: %lu\n", a->used, __memory);
    assert(aa_get(a, 1000, &value) == 0);
    printf("a[1000]: %d\n", value);

    aa_delete(a);

    assert(__memory == 0);

    return 0;
}

#endif /* TEST_AA_INT */
