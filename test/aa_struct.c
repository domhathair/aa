#include "alloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef TEST_AA_STRUCT

struct person {
    size_t age, height;
    bool gender;
};

enum { MALE = false, FEMALE = true };

#define AA_KEY char *
#define AA_VALUE struct person
#define AA_IMPLEMENTATION
#include "aa.h"

int main(void) {
    struct aa *a = aa_new();
    assert(a != NULL);

    aa_set(a, "Alexander", (const struct person){.age = 26, .height = 180, .gender = MALE});
    aa_set(a, "Valentina", (const struct person){.age = 27, .height = 153, .gender = FEMALE});
    aa_set(a, "Maria", (const struct person){.age = 26, .height = 172, .gender = FEMALE});

    for (size_t i = 0; i < aa_len(a); i++)
        if (a->buckets[i].entry != NULL)
            printf("%s -> age: %zu, height: %zu\n", a->buckets[i].entry->key, a->buckets[i].entry->value.age,
                   a->buckets[i].entry->value.height);

    printf("Maria is %zu years old\n", aa_get(a, "Maria").age);
    printf("Alexander's height is %zu\n", aa_get(a, "Alexander").height);
    printf("Valentina is %s\n", aa_get(a, "Valentina").age == MALE ? "male" : "female");

    printf("Heap of a[%zu]: %lu\n", a->used, __memory);

    aa_remove(a, "Maria");
    printf("Maria is %s\n", aa_get(a, "Maria").age == 0 ? "gone...\vThink about Maria...\vRegret..." : "still here!");

    aa_free(a);

    assert(__memory == 0);

    return 0;
}

#endif /* TEST_AA_STRUCT */
