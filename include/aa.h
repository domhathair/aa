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

#ifndef SIZE_WIDTH
#define SIZE_WIDTH (sizeof(size_t) * CHAR_BIT)
#endif /* SIZE_WIDTH */

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
    AA_HASH_FILLED = 1UL << (SIZE_WIDTH - 1)
};

#ifndef IS_POINTER
#if defined(__GNUC__) || defined(__clang__)
#ifndef POINTER_TYPE_CLASS
#define POINTER_TYPE_CLASS 5
#endif /* POINTER_TYPE_CLASS */

/* clang-format off */
#define IS_SAME_TYPE(a, b) (__builtin_types_compatible_p(typeof_unqual(a), typeof_unqual(b)))
#define IS_PTR_OR_ARRAY(p) (__builtin_classify_type(p) == POINTER_TYPE_CLASS)
#define DECAY(a)           (&*__builtin_choose_expr(IS_PTR_OR_ARRAY(a), a, NULL))
#define IS_POINTER(p)      (IS_SAME_TYPE(p, DECAY(p)))
/* clang-format on */
#else
#error "Not implemented"
#endif /* __GNUC__ || __clang__ */
#endif /* IS_POINTER */

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
 * @return A pointer to the newly created hash table, or NULL if the allocation fails
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
#define aa_get(aa, key, value) aa_x_get(aa, key, IS_POINTER(value) ? value : (typeof_unqual(value))NULL)

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
 * @brief Gets the number of active (non-deleted) entries in the hash table
 *
 * @param aa A pointer to the hash table
 * @return The number of active entries in the hash table, or 0 if the table is empty or invalid
 */
extern size_t aa_len(struct aa *);

/**
 * @brief Gets the number of key-value pairs in the hash table
 *
 * @param aa A pointer to the hash table
 * @return The number of key-value pairs in the hash table
 */
extern size_t aa_entries(struct aa *);

/**
 * @brief Iterates over the entries in the hash table
 *
 * @param aa A pointer to the hash table
 * @return A pointer to the next aa_node in the hash table, or NULL if no more entries
 */
extern struct aa_node *aa_next(struct aa *);

extern int aa_x_set(struct aa *, ... /* key, value */);
extern int aa_x_get(struct aa *, ... /* key, &value */);
extern int aa_x_remove(struct aa *, ... /* key */);

#endif /* AA_H */

#ifdef AA_IMPLEMENTATION

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

static int aa_alloc_htable(struct aa *a, size_t s) {
    if (!a || s == 0)
        return -1;

    struct aa_bucket *_Htable = (struct aa_bucket *)fat_malloc(sizeof(struct aa_bucket) * s);
    if (!_Htable)
        return -1;
    a->buckets = _Htable;

    return 0;
}

static int aa_init_table_if_needed(struct aa *a) {
    if (!a)
        return -1;

    if (!a->buckets)
        if (aa_alloc_htable(a, AA_INIT_NUM_BUCKETS) != 0)
            return -1;

    return 0;
}

static bool aa_empty(struct aa_bucket *b) {
    if (!b)
        return false;

    return b->hash == AA_HASH_EMPTY;
}

static bool aa_deleted(struct aa_bucket *b) {
    if (!b)
        return true;

    return b->hash == AA_HASH_DELETED;
}

static bool aa_filled(struct aa_bucket *b) {
    if (!b)
        return false;

    return b->hash & AA_HASH_FILLED;
}

static size_t aa_dim(struct aa_bucket *b) {
    if (!b)
        return 0;

    return fat_len(b) / sizeof(struct aa_bucket);
}

extern size_t aa_len(struct aa *a) {
    if (!a)
        return 0;

    if (a->deleted > a->used)
        return 0;

    return a->used - a->deleted;
}

static size_t aa_mask(struct aa *a) {
    if (!a)
        return 0;

    return aa_dim(a->buckets) - 1;
}

static struct aa_bucket *aa_find_slot_insert(struct aa *a, size_t hash) {
    if (!a || !a->buckets)
        return NULL;

    for (size_t m = aa_mask(a), i = hash & m, j = 1;; j++) {
        if (!aa_filled(&a->buckets[i]))
            return &a->buckets[i];

        i = (i + j) & m;
    }
}

static inline bool aa_equals(aa_key_t k1, aa_key_t k2) {
    if (IS_POINTER(k2))
        return strcmp((const char *)k1, (const char *)k2) == 0;
    else
        return k1 == k2;
}

static struct aa_bucket *aa_find_slot_lookup(struct aa *a, size_t hash, aa_key_t key) {
    if (!a || !a->buckets)
        return NULL;

    for (size_t m = aa_mask(a), i = hash & m, j = 1;; j++) {
        if (aa_empty(&a->buckets[i]))
            return NULL;

        if (a->buckets[i].hash == hash && aa_equals(key, a->buckets[i].entry->key))
            return &a->buckets[i];

        i = (i + j) & m;
    }
}

static size_t aa_bsr(size_t v) {
    if (v == 0)
        return 0;

    size_t bit_position = SIZE_WIDTH;
#ifdef _STDBIT_H
    return bit_position - stdc_first_leading_one(v);
#else
    for (; bit_position > 0; bit_position--)
        if ((v & ((size_t)1 << (bit_position - 1))) != 0)
            return bit_position - 1;

    return 0;
#endif /* _STDBIT_H */
}

static size_t aa_nextpow2(size_t n) {
    if (n == 0)
        return 1;

    const bool is_power_of2 = !((n - 1) & n);
    return 1 << (aa_bsr(n) + !is_power_of2);
}

static size_t aa_fnv1a(const void *data, size_t len) {
    const unsigned char *bytes = (const unsigned char *)data;
    enum {
#if SIZE_WIDTH == 128
        FNV_OFFSET_BASIS = 144066263297769815596495629667062367629U,
        FNV_PRIME = 309485009821345068724781371U,
#elif SIZE_WIDTH == 64
        FNV_OFFSET_BASIS = 14695981039346656037U,
        FNV_PRIME = 1099511628211U,
#elif SIZE_WIDTH == 32
        FNV_OFFSET_BASIS = 2166136261U,
        FNV_PRIME = 16777619U,
#else
#error "Not implemented"
#endif
    };

    size_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

static size_t aa_calc_hash(aa_key_t key) {
    /* clang-format off */
    size_t hash = !IS_POINTER(key)
        ? aa_fnv1a(&key, sizeof(key))
        : aa_fnv1a((const void *)key, strlen((const char *)key));
    /* clang-format on */

    return hash | AA_HASH_FILLED;
}

static void aa_clear_entry(struct aa_bucket *b) {
    if (!b || !b->entry)
        return;

    if ((void *)b->entry->key && IS_POINTER(b->entry->key))
        fat_free((void *)b->entry->key);
    fat_free(b->entry);

    b->entry = NULL;

    return;
}

static int aa_resize(struct aa *a, size_t s) {
    if (!a || s == 0)
        return -1;

    struct aa_bucket *o = a->buckets;
    if (aa_alloc_htable(a, s) != 0)
        return -1;

    for (size_t i = 0; i < aa_dim(o); i++) {
        struct aa_bucket *ob = &o[i];
        if (aa_filled(ob)) {
            struct aa_bucket *nb = aa_find_slot_insert(a, ob->hash);
            if (nb)
                *nb = *ob;
        } else if (aa_empty(ob) || aa_deleted(ob))
            aa_clear_entry(ob);
    }

    a->used -= a->deleted;
    a->deleted = 0;

    if (o)
        fat_free(o);

    return 0;
}

static int aa_grow(struct aa *a) {
    if (!a || !a->buckets)
        return -1;

    /* clang-format off */
    size_t s = aa_len(a) * AA_SHRINK_DEN < AA_GROW_FAC * aa_dim(a->buckets) * AA_SHRINK_NUM
            ? aa_dim(a->buckets)
            : (AA_GROW_FAC * aa_dim(a->buckets));
    /* clang-format on */

    return aa_resize(a, s);
}

static int aa_shrink(struct aa *a) {
    if (!a || !a->buckets)
        return -1;

    if (aa_dim(a->buckets) > AA_INIT_NUM_BUCKETS)
        return aa_resize(a, aa_dim(a->buckets) / AA_GROW_FAC);

    return 0;
}

static int aa_assign_key_ptr(struct aa_node *p, void *key) {
    if (!p || !key)
        return -1;

    p->key = (aa_key_t)fat_malloc(strlen((const char *)key) + 1);
    if (!(char *)p->key)
        return -1;
    strcpy((char *)p->key, (const char *)key);

    return 0;
}

extern struct aa *aa_new(void) {
    struct aa *a = (struct aa *)fat_malloc(sizeof(struct aa));
    if (!a)
        return NULL;

    a->buckets = NULL;
    a->deleted = a->used = 0;

    return a;
}

extern void aa_delete(struct aa *a) {
    if (!a)
        return;

    aa_clear(a), fat_free(a);

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

    if (aa_init_table_if_needed(a) != 0)
        return -1;

    const size_t hash = aa_calc_hash(key);
    struct aa_bucket *b = aa_find_slot_lookup(a, hash, key);

    if (b && b->entry) {
        b->entry->value = value;
        return 0;
    }

    b = aa_find_slot_insert(a, hash);
    if (!b)
        return -1;

    if (aa_deleted(b) && a->deleted > 0)
        a->deleted--;
    else if (++a->used * AA_GROW_DEN > aa_dim(a->buckets) * AA_GROW_NUM) {
        if (aa_grow(a) != 0)
            return -1;
        b = aa_find_slot_insert(a, hash);
    }

    if (aa_deleted(b)) {
        if (!IS_POINTER(b->entry->key))
            b->entry->key = key;
        else {
            fat_free((void *)b->entry->key);
            if (aa_assign_key_ptr(b->entry, (void *)key) != 0)
                return -1;
        }
        b->entry->value = value;
    } else {
        struct aa_node *n = (struct aa_node *)fat_malloc(sizeof(struct aa_node));
        if (!n)
            return -1;

        if (!IS_POINTER(n->key))
            n->key = key;
        else if (aa_assign_key_ptr(n, (void *)key) != 0) {
            fat_free(n);
            return -1;
        }
        n->value = value;

        b->entry = n;
    }

    b->hash = hash;

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

    struct aa_bucket *b = aa_find_slot_lookup(a, aa_calc_hash(key), key);
    if (b && aa_filled(b)) {
        if (value)
            *value = b->entry->value;
        return 0;
    }

    return -1;
}

extern int aa_rehash(struct aa *a) {
    if (!a)
        return -1;

    if (aa_len(a) != 0)
        return aa_resize(a, aa_nextpow2(AA_INIT_DEN * aa_len(a) / AA_INIT_NUM));

    return 0;
}

extern int aa_x_remove(struct aa *a, ...) {
    va_list args;
    va_start(args);
    aa_key_t key = va_arg(args, aa_key_t);
    va_end(args);

    if (!a)
        return -1;

    if (aa_len(a) == 0)
        return -1;

    const size_t hash = aa_calc_hash(key);
    struct aa_bucket *p = aa_find_slot_lookup(a, hash, key);
    if (p) {
        p->hash = AA_HASH_DELETED;

        a->deleted++;
        if (aa_len(a) == 0)
            aa_clear(a);
        else if (aa_len(a) * AA_SHRINK_DEN < aa_dim(a->buckets) * AA_SHRINK_NUM)
            if (aa_shrink(a) != 0)
                return -1;

        return 0;
    }

    return -1;
}

extern void aa_clear(struct aa *a) {
    if (!a || !a->buckets)
        return;

    for (size_t i = 0; i < aa_dim(a->buckets); i++)
        aa_clear_entry(&a->buckets[i]);

    fat_free(a->buckets);
    a->buckets = NULL;
    a->deleted = a->used = 0;

    return;
}

extern size_t aa_entries(struct aa *a) {
    if (!a || !a->buckets)
        return 0;

    return aa_dim(a->buckets);
}

extern struct aa_node *aa_next(struct aa *a) {
    static size_t i = 0;
    static struct aa *prev = NULL;

    /* Push NULL if you want to reset the iterator */
    if (prev != a) {
        i = 0;
        prev = a;
    }

    if (!a || !a->buckets)
        return NULL;

    size_t len = aa_entries(a);
    for (; i < len; i++)
        if (a->buckets[i].hash & AA_HASH_FILLED)
            if (a->buckets[i].entry)
                return a->buckets[i++].entry;

    i = 0;
    return NULL;
}

#endif /* AA_IMPLEMENTATION */
