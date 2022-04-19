#ifndef BIGNUM_H
#define BIGNUM_H

#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

/**
 * @brief store bignumber with dynamic size
 * number[size - 1] is the msb
 */
typedef struct _BigN {
    unsigned int *number;
    unsigned int size;
    int sign;
} BigN;

/* alloc struct BigN with the given size */
static inline BigN *alloc_BigN(size_t size);

/* transform bits into string */
static inline char *to_string_BigN(BigN *a);

/* |c| = |a| + |b| */
static inline void add_BigN(BigN *a, BigN *b, BigN *c);

/* |c| = |a| * |b| */
static inline void mul_BigN(BigN *a, BigN *b, BigN *c);

/* bitwise Left Shift BigN a->number by a constant bits (1~31) */
static inline void shl_BigN(BigN *a, const int bits);

/* copy a into b */
static inline void cpy_BigN(BigN *a, BigN *b);

/* resize a->size and reallocate a->number */
static inline void resize_BigN(BigN *a, size_t size);

/* calculate fib number without fast doubling */
static inline void fib_BigN(BigN *dest, int fn);

/* free BigN */
static inline void free_BigN(BigN *a);

/* swap BigN a b */
static inline void swap_BigN(BigN *a, BigN *b);

void do_add_BigN(BigN *a, BigN *b, BigN *c);


#endif