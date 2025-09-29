#include "alloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef TEST_AA_IS_POINTER

#define AA_KEY int
#define AA_VALUE int
#define AA_IMPLEMENTATION
#include "aa.h"

int main(void) {
    struct aa_is_pointer {
        int a;
    };

    bool a = false;
    unsigned char b = 0;
    signed char c = 0;
    unsigned short d = 0;
    signed short e = 0;
    unsigned int f = 0;
    signed int g = 0;
    unsigned long h = 0;
    signed long i = 0;
    unsigned long long j = 0;
    signed long long k = 0;
    float l = 0;
    double m = 0;
    long double n = 0;
    unsigned char *o = u8"1";
    char p[] = "1";
    char *q = "1";
    struct aa_is_pointer r = {0};
    struct aa_is_pointer *s = NULL;
    bool *t = NULL;

    assert(IS_POINTER(a) == 0);
    assert(IS_POINTER(b) == 0);
    assert(IS_POINTER(c) == 0);
    assert(IS_POINTER(d) == 0);
    assert(IS_POINTER(e) == 0);
    assert(IS_POINTER(f) == 0);
    assert(IS_POINTER(g) == 0);
    assert(IS_POINTER(h) == 0);
    assert(IS_POINTER(i) == 0);
    assert(IS_POINTER(j) == 0);
    assert(IS_POINTER(k) == 0);
    assert(IS_POINTER(l) == 0);
    assert(IS_POINTER(m) == 0);
    assert(IS_POINTER(n) == 0);
    assert(IS_POINTER(o) == 1);
    assert(IS_POINTER(p) == 0);
    assert(IS_POINTER(q) == 1);
    assert(IS_POINTER(r) == 0);
    assert(IS_POINTER(s) == 1);
    assert(IS_POINTER(t) == 1);
}

#endif /* TEST_AA_IS_POINTER */
