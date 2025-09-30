#include "alloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#ifdef TEST_AA_CHAR

#define AA_KEY char *
#define AA_VALUE char *
#define AA_IMPLEMENTATION
#include "aa.h"

int main(void) {
    struct aa *a = aa_new();
    assert(a);

    aa_set(a, "Stevie", "Ray Vaughan");
    aa_set(a, "Asım Can", "Gündüz");
    aa_set(a, "Dan", "Patlansky");
    aa_set(a, "İlter", "Kurcala");
    aa_set(a, "Кириллица", "Тоже работает");
    aa_set(a, "Ferhat", "Kurtulmuş");

    for (struct aa_node *node = NULL; (node = aa_next(a));)
        printf("%s -> %s\n", node->key, node->value);

    aa_value_t value;
    if (aa_get(a, "Dan", &value) != 0)
        printf("Dan does not exist!\n");
    else
        printf("Dan %s exists!\n", value);

    assert(aa_remove(a, "Ferhat") == 0);
    assert(aa_get(a, "Ferhat", &value) != 0);
    assert(aa_remove(a, "Foe") != 0);
    assert(aa_get(a, "İlter", &value) == 0);
    assert(strcmp(value, "Kurcala") == 0);

    aa_rehash(a);

    printf("%s\n", aa_get(a, "Stevie", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Asım Can", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Dan", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Кириллица", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Ferhat", &value) == 0 ? value : "(null)");
    printf("%s\n", aa_get(a, "Что-то, чего точно нет", &value) == 0 ? value : "(null)");

    printf("After rehash:\n");
    for (struct aa_node *node = NULL; (node = aa_next(a));)
        printf("%s -> %s\n", node->key, node->value);

    aa_delete(a);

    assert(__memory == 0);

    return 0;
}

#endif /* TEST_AA_CHAR */
