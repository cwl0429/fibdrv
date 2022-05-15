#ifndef BIGNUM_H
#define BIGNUM_H

#include <asm/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define DETECT_OVERFLOW(x, y) ((x) > (~y) ? 1 : 0)
#define DETECT_OVERFLOW_SUB(x, y) ((x) < (y) ? 1 : 0)
/**
 * @brief store bignumber with dynamic size
 * number[size - 1] is the msb
 */
typedef struct _bign {
    unsigned int *number;
    unsigned int size;
    /* sign is not use now */
    int sign;
} bign;

/* alloc struct bign with the given size */
static inline bign *bign_alloc(size_t size);

/* transform bits into string */
static inline char *bign_to_string(bign *a);

/* |c| = |a| + |b| */
static inline void bign_add(bign *a, bign *b, bign *c);

/* c = a - b */
static inline void bign_sub(bign *a, bign *b, bign *c);

/* |c| = |a| * |b| */
static inline void bign_mul(bign *a, bign *b, bign *c);

/* bitwise Left Shift bign a->number by a constant bits (1~31) */
static inline void bign_shl(bign *a, const int bits);

/* copy a into b */
static inline void bign_cpy(bign *a, bign *b);

/* resize a->size and reallocate a->number */
static inline void bign_resize(bign *a, size_t size);

/* calculate fib number without fast doubling */
static inline void bign_fib(bign *dest, int fn);

/* calculate fib number with fast doubling */
static inline void bign_fib_fast(bign *dest, const int fn);

/* free bign */
static inline void bign_free(bign *a);

/* swap bign a b */
static inline void bign_swap(bign *a, bign *b);

// /* add carry into a bign_mul_add*/
// void bign_mul_add(unsigned long long c, int offset, bign *a);


#endif