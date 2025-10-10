#include "alloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#ifdef TEST_AA_INT

#define AA_KEY int
#define AA_VALUE int
#define AA_IMPLEMENTATION
#include "aa.h"

int main(void) {
    printf("Allocator test\n");
    int *memory = (int *)fat_malloc(sizeof(int) * 1024);

    printf("Heap after allocation: %zu\n", _Allocated_memory);
    printf("Pointer length: %zu\n", fat_len(memory) / sizeof(int));

    assert(_Allocated_memory == fat_len(memory));

    fat_free(memory);
    printf("Heap after deallocation: %zu\n", _Allocated_memory);

    struct aa *a = aa_new();
    assert(a);

    for (int i = 0; i < 1000000; i++)
        assert(aa_set(a, i, i + 1) == 0);
    printf("Heap of a[%zu]: %zu\n", a->used, _Allocated_memory);

    for (int i = 2000; i < 1000000; i++)
        assert(aa_remove(a, i) == 0);
    aa_rehash(a);

    aa_value_t value;
    printf("Heap of a[%zu]: %zu\n", a->used, _Allocated_memory);
    assert(aa_get(a, 1000, &value) == 0);
    printf("a[1000]: %d\n", value);

    aa_delete(a);

    assert(_Allocated_memory == 0);

    return 0;
}

#endif /* TEST_AA_INT */
