/*
 * aa.h - A simple hash table (associative array) library
 *
 * The author of the original implementation: Martin Nowak
 *
 * Copyright (c) 2025, Ferhat Kurtulmuş, Alexander Chepkov
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS 'AS IS' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef aa_h
#define aa_h

#include "alloc.h"
#include "crc.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    /* Grow threshold */
    AA_GROW_NUM = 4,
    AA_GROW_DEN = 5,
    /* Shrink threshold */
    AA_SHRINK_NUM = 1,
    AA_SHRINK_DEN = 8,
    /* Grow factor */
    AA_GROW_FAC = 4,

    /* Initial load factor (for literals), mean of both thresholds */
    AA_INIT_NUM = (AA_GROW_DEN * AA_SHRINK_NUM + AA_GROW_NUM * AA_SHRINK_DEN) / 2,
    AA_INIT_DEN = AA_SHRINK_DEN * AA_GROW_DEN,

    AA_INIT_NUM_BUCKETS = 8,

    /* Magic hash constants to distinguish empty, deleted, and filled buckets */
    AA_HASH_EMPTY = 0,
    AA_HASH_DELETED = 1,
    AA_HASH_FILLED_MARK = (size_t)1 << (CHAR_BIT * sizeof(size_t) - 1)
};

struct aa_node {
#ifndef AA_KEY
#define AA_KEY size_t
#endif /* AA_KEY */
    AA_KEY key;

#ifndef AA_VALUE
#define AA_VALUE void *
#endif /* AA_VALUE */
    AA_VALUE value;
};

#if __STDC_VERSION__ >= 201904L && __STDC_VERSION__ < 202000L
#define AA_K typeof((struct aa_node){}.key)
#elif __STDC_VERSION__ >= 202000L
#define AA_K typeof_unqual((struct aa_node){}.key)
#if !defined(_WIN32) && !defined(_WIN64)
#include <stdbit.h>
#endif /* _WIN32 && _WIN64 */
#else
#error "C23 or later required"
#endif /* __STDC_VERSION__ */
#define AA_V AA_VALUE

#ifndef SIZE_WIDTH
#define SIZE_WIDTH 64
[[maybe_unused]] static void test_size_width(void) {
    (void)sizeof(char[(sizeof(size_t) * CHAR_BIT == SIZE_WIDTH) > 0 ? 1 : -1]);
    return;
}
#endif /* SIZE_WIDTH */

struct aa_bucket {
    size_t hash;
    struct aa_node *entry;
};

struct aa {
    struct aa_bucket *buckets;
    size_t first, used, deleted;
};

extern struct aa *aa_new(void);
extern void aa_free(struct aa *);
extern int aa_set(struct aa *, AA_K, AA_V);
extern AA_V aa_get(struct aa *, AA_K);
extern int aa_rehash(struct aa *);
extern bool aa_remove(struct aa *, AA_K);
extern void aa_clear(struct aa *);
extern size_t aa_len(struct aa *);

#endif /* aa_h */

#ifdef AA_IMPLEMENTATION

#define IS_POINTER(x)                                                                                                  \
    _Generic((x),                                                                                                      \
        bool: 0,                                                                                                       \
        unsigned char: 0,                                                                                              \
        signed char: 0,                                                                                                \
        unsigned short: 0,                                                                                             \
        signed short: 0,                                                                                               \
        unsigned int: 0,                                                                                               \
        signed int: 0,                                                                                                 \
        unsigned long: 0,                                                                                              \
        signed long: 0,                                                                                                \
        unsigned long long: 0,                                                                                         \
        signed long long: 0,                                                                                           \
        float: 0,                                                                                                      \
        double: 0,                                                                                                     \
        long double: 0,                                                                                                \
        unsigned char *: 1,                                                                                            \
        signed char *: 1,                                                                                              \
        char *: 1,                                                                                                     \
        default: assert(0))

static int alloc_htable(struct aa *a, size_t sz) {
    if (a == NULL)
        return -1;

    struct aa_bucket *__htable = (struct aa_bucket *)__malloc(sizeof(struct aa_bucket) * sz);
    if (__htable == NULL)
        return -1;
    a->buckets = __htable;

    return 0;
}

static int init_table_if_needed(struct aa *a) {
    if (a == NULL)
        return -1;

    if (a->buckets == NULL) {
        if (alloc_htable(a, AA_INIT_NUM_BUCKETS) != 0)
            return -1;

        a->first = AA_INIT_NUM_BUCKETS;
    }

    return 0;
}

static bool empty(struct aa_bucket *b) {
    if (b == NULL)
        return false;

    return b->hash == AA_HASH_EMPTY;
}

static bool deleted(struct aa_bucket *b) {
    if (b == NULL)
        return true;

    return b->hash == AA_HASH_DELETED;
}

static bool filled(struct aa_bucket *b) {
    if (b == NULL)
        return false;

    return (ptrdiff_t)b->hash < 0;
}

static size_t dim(struct aa_bucket *b) {
    if (b == NULL)
        return 0;

    return __len(b) / sizeof(struct aa_bucket);
}

static size_t len(struct aa *a) {
    if (a == NULL)
        return 0;

    if (a->deleted > a->used)
        return 0;

    return a->used - a->deleted;
}

static size_t mask(struct aa *a) {
    if (a == NULL)
        return 0;

    return dim(a->buckets) - 1;
}

static struct aa_bucket *find_slot_insert(struct aa *a, size_t hash) {
    if (a == NULL)
        return NULL;

    for (size_t m = mask(a), i = hash & m, j = 1;; j++) {
        if (filled(&a->buckets[i]) == false)
            return &a->buckets[i];

        i = (i + j) & m;
    }
}

static inline bool equals(AA_K k1, AA_K k2) {
    if (IS_POINTER(k2) == 0)
        return k1 == k2;
    else
        return strcmp((const char *)k1, (const char *)k2) == 0;
}

static struct aa_bucket *find_slot_lookup(struct aa *a, size_t hash, AA_K key) {
    if (a == NULL)
        return NULL;

    for (size_t m = mask(a), i = hash & m, j = 1;; j++) {
        if (a->buckets[i].hash == hash && equals(key, a->buckets[i].entry->key))
            return &a->buckets[i];

        if (empty(&a->buckets[i]))
            return NULL;

        i = (i + j) & m;
    }
}

static size_t bsr(size_t v) {
    if (v == 0)
        return 0;

    size_t bit_position = SIZE_WIDTH;
#if __STDC_VERSION__ >= 202000L && defined(_STDBIT_H)
    return bit_position - stdc_first_leading_one(v);
#else
    for (; bit_position > 0; bit_position--)
        if ((v & ((size_t)1 << (bit_position - 1))) != 0)
            return bit_position - 1;

    return 0;
#endif /* __STDC_VERSION__ */
}

static size_t nextpow2(size_t n) {
    if (n == 0)
        return 1;

    const bool is_power_of2 = !((n - 1) & n);
    return 1 << (bsr(n) + !is_power_of2);
}

static size_t mix(size_t h) {
    enum { m = 0x5BD1E995 };
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}

static size_t calc_hash(AA_K key) {
#define CRC CRC_TRANSITIVE(SIZE_WIDTH)
#define CRC_TRANSITIVE(width) CRC_IMPLEMENTATION(width)
#define CRC_IMPLEMENTATION(width)                                                                                      \
    IS_POINTER(key) == 0 ? crc##width(SIZE_MAX, &key, sizeof(key))                                                     \
                         : crc##width(SIZE_MAX, (const char *)key, strlen((const char *)key))

    return mix(CRC) | AA_HASH_FILLED_MARK;
}

static void clear_entry(struct aa_bucket *b) {
    if (b == NULL)
        return;

    if (b->entry == NULL)
        return;

    if ((void *)b->entry->key != NULL && IS_POINTER(b->entry->key) != 0)
        __free((void *)b->entry->key);
    __free(b->entry);

    b->entry = NULL;

    return;
}

static int resize(struct aa *a, size_t sz) {
    if (a == NULL)
        return -1;

    struct aa_bucket *obuckets = a->buckets;
    if (alloc_htable(a, sz) != 0)
        return -1;

    for (size_t i = a->first; i < dim(obuckets); i++) {
        struct aa_bucket *b = &obuckets[i];
        if (filled(b) == true)
            *find_slot_insert(a, b->hash) = *b;
        if (empty(b) == true || deleted(b) == true)
            clear_entry(b);
    }

    a->first = 0;
    a->used -= a->deleted;
    a->deleted = 0;

    __free(obuckets);

    return 0;
}

static int grow(struct aa *a) {
    if (a == NULL)
        return -1;

    size_t sz = /*  */
        len(a) * AA_SHRINK_DEN < AA_GROW_FAC * dim(a->buckets) * AA_SHRINK_NUM ? dim(a->buckets)
                                                                               : AA_GROW_FAC * dim(a->buckets);

    return resize(a, sz);
}

static int shrink(struct aa *a) {
    if (a == NULL)
        return -1;

    if (dim(a->buckets) > AA_INIT_NUM_BUCKETS)
        return resize(a, dim(a->buckets) / AA_GROW_FAC);

    return 0;
}

static int assign_key_ptr(struct aa_node *p, void *key) {
    if (p == NULL || key == NULL)
        return -1;

    p->key = (AA_K)__malloc(strlen((const char *)key) + 1);
    if ((void *)p->key == NULL)
        return -1;
    strcpy((char *)p->key, (const char *)key);

    return 0;
}

extern struct aa *aa_new(void) {
    struct aa *a = (struct aa *)__malloc(sizeof(struct aa));
    if (a == NULL) {
#ifndef NDEBUG
        assert(0);
#endif /* NDEBUG */
        return NULL;
    }

    a->buckets = NULL;
    a->first = a->deleted = a->used = 0;

    return a;
}

extern void aa_free(struct aa *a) {
    if (a == NULL)
        return;

    aa_clear(a), __free(a);

    return;
}

extern int aa_set(struct aa *a, AA_K key, AA_V value) {
    if (a == NULL)
        return -1;

    if (init_table_if_needed(a) != 0)
        return -1;

    const size_t key_hash = calc_hash(key);

    struct aa_bucket *p = find_slot_lookup(a, key_hash, key);
    if (p != NULL) {
        p->entry->value = value;
        return 0;
    }
    p = find_slot_insert(a, key_hash);
    if (p == NULL)
        return -1;

    if (deleted(p) == true)
        a->deleted--;
    else if (++a->used * AA_GROW_DEN > dim(a->buckets) * AA_GROW_NUM) {
        if (grow(a) != 0)
            return -1;
        p = find_slot_insert(a, key_hash);
    }

    size_t m = (size_t)(p - a->buckets);
    if (m < a->first)
        a->first = m;

    p->hash = key_hash;

    if (deleted(p) == true) {
        if (IS_POINTER(p->entry->key) == 0)
            p->entry->key = key;
        else {
            __free((void *)p->entry->key);
            if (assign_key_ptr(p->entry, (void *)key) != 0)
                return -1;
        }
        p->entry->value = value;
    } else {
        struct aa_node *new_node = (struct aa_node *)__malloc(sizeof(struct aa_node));
        if (new_node == NULL)
            return -1;

        if (IS_POINTER(new_node->key) == 0)
            new_node->key = key;
        else if (assign_key_ptr(new_node, (void *)key) != 0)
            return -1;
        new_node->value = value;

        p->entry = new_node;
    }

    return 0;
}

extern AA_V aa_get(struct aa *a, AA_K key) {
    if (a == NULL)
        return (struct aa_node){.key = (AA_K)NULL}.value;

    struct aa_bucket *b = find_slot_lookup(a, calc_hash(key), key);
    if (b != NULL && filled(b) == true)
        return b->entry->value;

    return (struct aa_node){.key = (AA_K)NULL}.value;
}

extern int aa_rehash(struct aa *a) {
    if (a == NULL)
        return -1;

    if (len(a) != 0)
        return resize(a, nextpow2(AA_INIT_DEN * len(a) / AA_INIT_NUM));

    return 0;
}

extern bool aa_remove(struct aa *a, AA_K key) {
    if (a == NULL)
        return false;

    if (len(a) == 0)
        return false;

    const size_t hash = calc_hash(key);
    struct aa_bucket *p = find_slot_lookup(a, hash, key);
    if (p != NULL) {
        p->hash = AA_HASH_DELETED;

        ++a->deleted;
        if (len(a) * AA_SHRINK_DEN < dim(a->buckets) * AA_SHRINK_NUM)
            if (shrink(a) != 0)
                return false;

        return true;
    }
    return false;
}

extern void aa_clear(struct aa *a) {
    if (a == NULL)
        return;

    for (size_t i = 0; i < dim(a->buckets); i++) clear_entry(&a->buckets[i]);

    __free(a->buckets);
    a->buckets = NULL;
    a->first = a->deleted = a->used = 0;

    return;
}

extern size_t aa_len(struct aa *a) {
    if (a == NULL)
        return 0;

    if (a->buckets == NULL)
        return 0;

    return dim(a->buckets);
}

#endif /* AA_IMPLEMENTATION */
