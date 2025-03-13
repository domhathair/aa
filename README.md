## aa.h - A Simple Hash Table (Associative Array) Library

This library provides a simple implementation of a hash table (associative array) in C.  
It allows for the storage and retrieval of key-value pairs with constant average time complexity for  
insertion, deletion, and lookup operations.

### Note
This library is essentially a direct port of [nogcaa](https://github.com/domhathair/nogcaa) to C.

### Compatibility
This library utilizes features specific to the C11 and C23 standards, such as `_Generic` and `typeof_unqual`.
It is recommended to use the latest versions of compilers that support these features to ensure compatibility.

### Features
- Dynamic resizing of the hash table to maintain performance.
- Support for custom key and value types.
- Efficient handling of hash collisions using open addressing.
- Automatic rehashing to maintain load factors.

### API

#### Data Structures
- `struct aa`: The main structure representing the hash table.
- `struct aa_node`: Represents a key-value pair in the hash table.
- `struct aa_bucket`: Represents a bucket in the hash table.

#### Functions
- `struct aa *aa_new(void)`: Creates a new hash table.
- `void aa_free(struct aa *a)`: Frees the memory allocated for the hash table.
- `int aa_set(struct aa *a, AA_KEY_T key, AA_VALUE_T value)`: Inserts or updates a key-value pair in the hash table.
- `AA_VALUE_T aa_get(struct aa *a, AA_KEY_T key)`: Retrieves the value associated with a key.
- `int aa_rehash(struct aa *a)`: Rehashes the hash table to improve performance.
- `bool aa_remove(struct aa *a, AA_KEY_T key)`: Removes a key-value pair from the hash table.
- `void aa_clear(struct aa *a)`: Clears all key-value pairs from the hash table.
- `size_t aa_len(struct aa *a)`: Returns the number of buckets in the hash table.

### Custom Key and Value Types
To use custom key and value types, define `AA_KEY` and `AA_VALUE` before including `aa.h`. For example:
```c
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

    aa_free(a);

    return 0;
}
```

## How to build PlatformIO based project

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Сlone this repository
3. Run these commands:

```shell
# Build project
$ pio run

# Run tests
$ pio test --verbose --environment linux

# Clean build files
$ pio run --target clean
```
