#include "alloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#ifdef TEST_AA_STRUCT

enum gender : bool { MALE, FEMALE };

struct person {
    unsigned char age, height;
    enum gender gender;
};

#define AA_KEY char *
#define AA_VALUE struct person
#define AA_IMPLEMENTATION
#include "aa.h"

int main(void) {
    struct aa *a = aa_new();
    assert(a);

    aa_set(a, "Alexander", ((const struct person){.age = 26, .height = 180, .gender = MALE}));
    aa_set(a, "Valentina", ((const struct person){.age = 27, .height = 153, .gender = FEMALE}));
    aa_set(a, "Maria", ((const struct person){.age = 26, .height = 172, .gender = FEMALE}));

    for (struct aa_node *node = NULL; (node = aa_next(a));)
        printf("%s -> age: %hhu, height: %hhu\n", node->key, node->value.age, node->value.height);

    aa_value_t value;
    assert(aa_get(a, "Maria", &value) == 0);
    printf("Maria is %hhu years old\n", value.age);
    assert(aa_get(a, "Alexander", &value) == 0);
    printf("Alexander's height is %hhu\n", value.height);
    assert(aa_get(a, "Valentina", &value) == 0);
    printf("Valentina is %s\n", value.gender == MALE ? "male" : "female");

    printf("Heap of a[%zu]: %zu\n", a->used, _Allocated_memory);

    assert(aa_remove(a, "Maria") == 0);
    printf("Maria is %s\n",
           aa_get(a, "Maria", &value) != 0 ? "gone...\vThink about Maria...\vRegret..." : "still here!");

    aa_delete(a);

    assert(_Allocated_memory == 0);

    return 0;
}

#endif /* TEST_AA_STRUCT */
