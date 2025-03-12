#include "alloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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

    AA_VALUE dan = aa_get(a, "Dan");
    if (dan != NULL)
        printf("Dan %s exists!\n", dan);
    else
        printf("Dan does not exist!\n");

    assert(aa_remove(a, "Ferhat") == true);
    assert(aa_get(a, "Ferhat") == NULL);
    assert(aa_remove(a, "Foe") == false);
    assert(strcmp(aa_get(a, "İlter"), "Kurcala") == 0);

    aa_rehash(a);

    char *p;
    printf("%s\n", (p = aa_get(a, "Stevie")) != NULL ? p : "(null)");
    printf("%s\n", (p = aa_get(a, "Asım Can")) != NULL ? p : "(null)");
    printf("%s\n", (p = aa_get(a, "Dan")) != NULL ? p : "(null)");
    printf("%s\n", (p = aa_get(a, "Кириллица")) != NULL ? p : "(null)");
    printf("%s\n", (p = aa_get(a, "Ferhat")) != NULL ? p : "(null)");
    printf("%s\n", (p = aa_get(a, "Что-то, чего точно нет")) != NULL ? p : "(null)");

    aa_free(a);

    assert(__memory == 0);

    return 0;
}
