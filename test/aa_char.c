#include "alloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef TEST_AA_CHAR

#define AA_KEY char *
#define AA_VALUE char *
#define AA_IMPLEMENTATION
#include "aa.h"

int main(void) {
    struct aa *a = aa_new();
    assert(a != NULL);

    aa_set(a, "Stevie", "Ray Vaughan");
    aa_set(a, "Asım Can", "Gündüz");
    aa_set(a, "Dan", "Patlansky");
    aa_set(a, "İlter", "Kurcala");
    aa_set(a, "Кириллица", "Тоже работает");
    aa_set(a, "Ferhat", "Kurtulmuş");

    for (size_t i = 0; i < aa_len(a); i++)
        if (a->buckets[i].entry != NULL)
            printf("%s -> %s\n", a->buckets[i].entry->key, a->buckets[i].entry->value);

    AA_VALUE value;
    if (aa_get(a, "Dan", &value) != 0)
        printf("Dan does not exist!\n");
    else
        printf("Dan %s exists!\n", value);

    assert(aa_remove(a, "Ferhat") == true);
    assert(aa_get(a, "Ferhat", &value) != 0);
    assert(aa_remove(a, "Foe") == false);
    assert(aa_get(a, "İlter", &value) == 0);
    assert(strcmp(value, "Kurcala") == 0);

    aa_rehash(a);

    printf("%s\n", aa_get(a, "Stevie", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Asım Can", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Dan", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Кириллица", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Ferhat", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Что-то, чего точно нет", &value) == 0 ? value : "(null)");

    aa_delete(a);

    assert(__memory == 0);

    return 0;
}

#endif /* TEST_AA_CHAR */
