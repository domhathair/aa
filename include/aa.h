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

#ifndef AA_H
#define AA_H

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "alloc.h"
#include "crc.h"

/**
 * @brief Enumeration for hash table constants
 */
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

/**
 * @brief Forward declaration of the aa_node structure
 */
struct aa_node;

/**
 * @brief Structure representing a bucket in the hash table
 */
struct aa_bucket {
    size_t hash;
    struct aa_node *entry;
};

/**
 * @brief Structure representing the hash table
 */
struct aa {
    struct aa_bucket *buckets;
    size_t used, deleted;
};

/**
 * @brief Creates a new hash table
 *
 * @return A pointer to the newly created hash table, or nullptr if the allocation fails
 */
extern struct aa *aa_new(void);

/**
 * @brief Deletes a hash table and frees its memory
 *
 * @param aa A pointer to the hash table to be deleted
 */
extern void aa_delete(struct aa *);

/**
 * @brief Sets a key-value pair in the hash table
 *
 * @param aa A pointer to the hash table
 * @param key The key to be set
 * @param value The value to be associated with the key
 * @return 0 on success, -1 on failure.
 */
#define aa_set(aa, key, value) aa_x_set(aa, key, value)

/**
 * @brief Gets the value associated with a key in the hash table
 *
 * @param aa A pointer to the hash table
 * @param key The key whose value is to be retrieved
 * @param value A pointer to the variable where the value will be stored
 * @return 0 on success, -1 on failure
 */
#define aa_get(aa, key, value) aa_x_get(aa, key, value)

/**
 * @brief Removes a key-value pair from the hash table
 *
 * @param aa A pointer to the hash table
 * @param key The key to be removed
 * @return 0 on success, -1 on failure
 */
#define aa_remove(aa, key) aa_x_remove(aa, key)

/**
 * @brief Rehashes the hash table to a new size
 *
 * @param aa A pointer to the hash table
 * @return 0 on success, -1 on failure
 */
extern int aa_rehash(struct aa *);

/**
 * @brief Clears all key-value pairs from the hash table
 *
 * @param aa A pointer to the hash table
 */
extern void aa_clear(struct aa *);

/**
 * @brief Gets the number of key-value pairs in the hash table
 *
 * @param a A pointer to the hash table
 * @return The number of key-value pairs in the hash table
 */
extern size_t aa_len(struct aa *);

extern int aa_x_set(struct aa *, ... /* key, value */);
extern int aa_x_get(struct aa *, ... /* key, &value */);
extern int aa_x_remove(struct aa *, ... /* key */);

#endif /* AA_H */

#ifdef AA_IMPLEMENTATION
#ifndef AA_IMPLEMENTED
#define AA_IMPLEMENTED

#ifndef AA_KEY
#error "Please define AA_KEY type"
#endif /* AA_KEY */

#ifndef AA_VALUE
#error "Please define AA_VALUE type"
#endif /* AA_VALUE */

#if __STDC_VERSION__ >= 202311L
typedef typeof_unqual(AA_KEY) aa_key_t;
typedef typeof_unqual(AA_VALUE) aa_value_t;
#if (__linux__)
#include <stdbit.h>
#endif /* __linux__ */
#else
#error "C23 or later required"
#endif /* __STDC_VERSION__ */

struct aa_node {
    aa_key_t key;
    aa_value_t value;
};

#ifndef SIZE_WIDTH
#define SIZE_WIDTH 64
[[maybe_unused]] static void test_size_width(void) {
    (void)sizeof(char[(sizeof(size_t) * CHAR_BIT == SIZE_WIDTH) > 0 ? 1 : -1]);
    return;
}
#endif /* SIZE_WIDTH */

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
    if (!a || sz == 0)
        return -1;

    struct aa_bucket *_Htable = (struct aa_bucket *)__malloc(sizeof(struct aa_bucket) * sz);
    if (!_Htable)
        return -1;
    a->buckets = _Htable;

    return 0;
}

static int init_table_if_needed(struct aa *a) {
    if (!a)
        return -1;

    if (!a->buckets)
        if (alloc_htable(a, AA_INIT_NUM_BUCKETS) != 0)
            return -1;

    return 0;
}

static bool empty(struct aa_bucket *b) {
    if (!b)
        return false;

    return b->hash == AA_HASH_EMPTY;
}

static bool deleted(struct aa_bucket *b) {
    if (!b)
        return true;

    return b->hash == AA_HASH_DELETED;
}

static bool filled(struct aa_bucket *b) {
    if (!b)
        return false;

    return (ptrdiff_t)b->hash < 0;
}

static size_t dim(struct aa_bucket *b) {
    if (!b)
        return 0;

    return __len(b) / sizeof(struct aa_bucket);
}

static size_t len(struct aa *a) {
    if (!a)
        return 0;

    if (a->deleted > a->used)
        return 0;

    return a->used - a->deleted;
}

static size_t mask(struct aa *a) {
    if (!a)
        return 0;

    return dim(a->buckets) - 1;
}

static struct aa_bucket *find_slot_insert(struct aa *a, size_t hash) {
    if (!a || !a->buckets)
        return nullptr;

    for (size_t m = mask(a), i = hash & m, j = 1;; j++) {
        if (!filled(&a->buckets[i]))
            return &a->buckets[i];

        i = (i + j) & m;
    }
}

static inline bool equals(aa_key_t k1, aa_key_t k2) {
    if (IS_POINTER(k2) == 0)
        return k1 == k2;
    else
        return strcmp((const char *)k1, (const char *)k2) == 0;
}

static struct aa_bucket *find_slot_lookup(struct aa *a, size_t hash, aa_key_t key) {
    if (!a || !a->buckets)
        return nullptr;

    for (size_t m = mask(a), i = hash & m, j = 1;; j++) {
        if (empty(&a->buckets[i]))
            return nullptr;

        if (a->buckets[i].hash == hash && equals(key, a->buckets[i].entry->key))
            return &a->buckets[i];

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

static size_t calc_hash(aa_key_t key) {
#define CRC CRC_TRANSITIVE(SIZE_WIDTH)
#define CRC_TRANSITIVE(width) CRC_IMPLEMENTATION(width)
#define CRC_IMPLEMENTATION(width)                                                                                      \
    IS_POINTER(key) == 0 ? crc##width(SIZE_MAX, &key, sizeof(key))                                                     \
                         : crc##width(SIZE_MAX, (const char *)key, strlen((const char *)key))

    return mix(CRC) | AA_HASH_FILLED_MARK;
}

static void clear_entry(struct aa_bucket *b) {
    if (!b || !b->entry)
        return;

    if ((void *)b->entry->key && IS_POINTER(b->entry->key) != 0)
        __free((void *)b->entry->key);
    __free(b->entry);

    b->entry = nullptr;

    return;
}

static int resize(struct aa *a, size_t sz) {
    if (!a || sz == 0)
        return -1;

    struct aa_bucket *obuckets = a->buckets;
    if (alloc_htable(a, sz) != 0)
        return -1;

    for (size_t i = 0; i < dim(obuckets); i++) {
        struct aa_bucket *ob = &obuckets[i];
        if (filled(ob)) {
            struct aa_bucket *nb = find_slot_insert(a, ob->hash);
            if (nb)
                *nb = *ob;
        } else if (empty(ob) || deleted(ob))
            clear_entry(ob);
    }

    a->used -= a->deleted;
    a->deleted = 0;

    __free(obuckets);

    return 0;
}

static int grow(struct aa *a) {
    if (!a || !a->buckets)
        return -1;

    size_t sz = /* \n */
        len(a) * AA_SHRINK_DEN < AA_GROW_FAC * dim(a->buckets) * AA_SHRINK_NUM ? dim(a->buckets)
                                                                               : AA_GROW_FAC * dim(a->buckets);

    return resize(a, sz);
}

static int shrink(struct aa *a) {
    if (!a || !a->buckets)
        return -1;

    if (dim(a->buckets) > AA_INIT_NUM_BUCKETS)
        return resize(a, dim(a->buckets) / AA_GROW_FAC);

    return 0;
}

static int assign_key_ptr(struct aa_node *p, void *key) {
    if (!p || !key)
        return -1;

    p->key = (aa_key_t)__malloc(strlen((const char *)key) + 1);
    if (!(char *)p->key)
        return -1;
    strcpy((char *)p->key, (const char *)key);

    return 0;
}

extern struct aa *aa_new(void) {
    struct aa *a = (struct aa *)__malloc(sizeof(struct aa));
    if (!a)
        return nullptr;

    a->buckets = nullptr;
    a->deleted = a->used = 0;

    return a;
}

extern void aa_delete(struct aa *a) {
    if (!a)
        return;

    aa_clear(a), __free(a);

    return;
}

extern int aa_x_set(struct aa *a, ...) {
    va_list args;
    va_start(args);
    aa_key_t key = va_arg(args, aa_key_t);
    aa_value_t value = va_arg(args, aa_value_t);
    va_end(args);

    if (!a)
        return -1;

    if (init_table_if_needed(a) != 0)
        return -1;

    const size_t hash = calc_hash(key);

    struct aa_bucket *p = find_slot_lookup(a, hash, key);
    if (p && p->entry) {
        p->entry->value = value;
        return 0;
    }

    p = find_slot_insert(a, hash);
    if (!p)
        return -1;

    if (deleted(p) && a->deleted > 0)
        a->deleted--;
    else if (++a->used * AA_GROW_DEN > dim(a->buckets) * AA_GROW_NUM) {
        if (grow(a) != 0)
            return -1;
        p = find_slot_insert(a, hash);
    }

    p->hash = hash;

    if (deleted(p)) {
        if (IS_POINTER(p->entry->key) == 0)
            p->entry->key = key;
        else {
            __free((void *)p->entry->key);
            if (assign_key_ptr(p->entry, (void *)key) != 0)
                return -1;
        }
        p->entry->value = value;
    } else {
        struct aa_node *nnode = (struct aa_node *)__malloc(sizeof(struct aa_node));
        if (!nnode)
            return -1;

        if (IS_POINTER(nnode->key) == 0)
            nnode->key = key;
        else if (assign_key_ptr(nnode, (void *)key) != 0) {
            __free(nnode);
            return -1;
        }
        nnode->value = value;

        p->entry = nnode;
    }

    return 0;
}

extern int aa_x_get(struct aa *a, ...) {
    va_list args;
    va_start(args);
    aa_key_t key = va_arg(args, aa_key_t);
    aa_value_t *value = va_arg(args, aa_value_t *);
    va_end(args);

    if (!a || !a->buckets)
        return -1;

    struct aa_bucket *b = find_slot_lookup(a, calc_hash(key), key);
    if (b && filled(b)) {
        if (value)
            *value = b->entry->value;
        return 0;
    }

    return -1;
}

extern int aa_rehash(struct aa *a) {
    if (!a)
        return -1;

    if (len(a) != 0)
        return resize(a, nextpow2(AA_INIT_DEN * len(a) / AA_INIT_NUM));

    return 0;
}

extern int aa_x_remove(struct aa *a, ...) {
    va_list args;
    va_start(args);
    aa_key_t key = va_arg(args, aa_key_t);
    va_end(args);

    if (!a)
        return -1;

    if (len(a) == 0)
        return -1;

    const size_t hash = calc_hash(key);
    struct aa_bucket *p = find_slot_lookup(a, hash, key);
    if (p) {
        p->hash = AA_HASH_DELETED;

        a->deleted++;
        if (len(a) * AA_SHRINK_DEN < dim(a->buckets) * AA_SHRINK_NUM)
            if (shrink(a) != 0)
                return -1;

        return 0;
    }

    return -1;
}

extern void aa_clear(struct aa *a) {
    if (!a || !a->buckets)
        return;

    for (size_t i = 0; i < dim(a->buckets); i++) /* \n */
        clear_entry(&a->buckets[i]);

    __free(a->buckets);
    a->buckets = nullptr;
    a->deleted = a->used = 0;

    return;
}

extern size_t aa_len(struct aa *a) {
    if (!a || !a->buckets)
        return 0;

    return dim(a->buckets);
}

#endif /* AA_IMPLEMENTED */
#endif /* AA_IMPLEMENTATION */
