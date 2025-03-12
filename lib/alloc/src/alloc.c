#include "alloc.h"
#include <stdlib.h>

size_t __memory = 0;

static struct fat_pointer *__get_ptr(void *ptr) {
    if (ptr == NULL || (size_t)ptr < offsetof(struct fat_pointer, ptr))
        return NULL;

    struct fat_pointer *__fat = (struct fat_pointer *)((size_t)ptr - offsetof(struct fat_pointer, ptr));
    if (__fat == NULL)
        return NULL;

    return __fat;
}

static size_t *__len_ptr(void *ptr) {
    struct fat_pointer *__fat = __get_ptr(ptr);
    if (__fat == NULL)
        return NULL;

    return &__fat->len;
}

extern size_t __len(void *ptr) {
    size_t *len = __len_ptr(ptr);
    if (len == NULL)
        return 0;

    return *len;
}

extern void *__malloc(size_t len) {
    if (len == 0)
        return NULL;
    else if (__memory + len > MAX_HEAP)
        return NULL;

    struct fat_pointer *__fat = (struct fat_pointer *)malloc(offsetof(struct fat_pointer, ptr) + len);
    if (__fat == NULL)
        return NULL;

    __memory += __fat->len = len;
    for (size_t i = 0; i < __fat->len; i++) (&__fat->ptr)[i] = 0;

    return &__fat->ptr;
}

extern void *__realloc(void *ptr, size_t len) {
    if (len == 0) {
        __free(ptr);
        return NULL;
    }

    if (ptr == NULL)
        return __malloc(len);

    size_t *len_ptr = __len_ptr(ptr);
    if (len_ptr == NULL)
        return ptr; /* Undefined behavior */

    if (len <= *len_ptr) {
        *len_ptr = len;
        return ptr;
    }

    void *ptr_new = __malloc(len);
    if (ptr_new == NULL)
        return NULL;

    for (size_t i = 0; i < *len_ptr; i++) /*  */
        ((unsigned char *)ptr_new)[i] = ((unsigned char *)ptr)[i];
    __free(ptr);

    return ptr_new;
}

extern void __free(void *ptr) {
    struct fat_pointer *__fat = __get_ptr(ptr);
    if (__fat == NULL)
        return;

    __memory -= __fat->len;
    for (size_t i = 0; i < __fat->len; i++) /*  */
        (&__fat->ptr)[i] = 0;

    free(__fat);

    return;
}
