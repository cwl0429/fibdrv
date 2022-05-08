
#include "bignum.h"


/*#ifndef SWAP
#define SWAP(x, y)           \
    do {                     \
        typeof(x) __tmp = x; \
        x = y;               \
        y = __tmp;           \
    } while (0)
#endif
*/


static inline bign *bign_alloc(size_t size)
{
    bign *bn = kmalloc(sizeof(bign), GFP_KERNEL);
    if (bn) {
        bn->number = kmalloc(sizeof(int) * size, GFP_KERNEL);
        bn->size = size;
        bn->sign = 0;
        if (bn->number) {
            memset(bn->number, 0, size);
            return bn;
        }
    }
    return NULL;
}

static inline char *bign_to_string(bign *a)
{
    /* change of base formula: log10(x) = log2(x) / log2(10) */
    size_t len = (8 * sizeof(int) * a->size) / 3 + 2;
    char *c = kmalloc(len, GFP_KERNEL);
    char *p = c;
    memset(c, '0', len - 1);
    c[len - 1] = '\0';

    /* transform bits into decimal string */
    for (int i = a->size - 1; i >= 0; i--) {
        for (unsigned int d = 1U << 31; d; d >>= 1) {
            int carry = !!(d & a->number[i]);
            for (int j = len - 2; j >= 0; j--) {
                c[j] += c[j] - '0' + carry;
                carry = (c[j] > '9');
                if (carry)
                    c[j] -= 10;
            }
        }
    }
    while (*p == '0' && *(p + 1) != '\0')
        p++;
    if (a->sign)
        *(--p) = '-';
    memmove(c, p, strlen(p) + 1);
    return c;
}

static inline void bign_add(bign *a, bign *b, bign *c)
{
    bign_resize(c, a->size + 1);
    unsigned int carry = 0;
    for (int i = 0; i < c->size; i++) {
        unsigned int tmp_a = (i < a->size) ? a->number[i] : 0;
        unsigned int tmp_b = (i < b->size) ? b->number[i] : 0;
        c->number[i] = tmp_a + tmp_b + carry;
        carry = DETECT_OVERFLOW(tmp_a, tmp_b + carry);
    }
    if (!c->number[c->size - 1] && c->size > 1)
        bign_resize(c, c->size - 1);
}

static inline void bign_sub(bign *a, bign *b, bign *c)
{
    bign_resize(c, a->size);
    unsigned int carry = 0;
    for (int i = 0; i < c->size; i++) {
        long tmp_a = (i < a->size) ? (long) a->number[i] : 0;
        long tmp_b = (i < b->size) ? (long) b->number[i] : 0;
        c->number[i] = tmp_a - tmp_b - carry;
        carry = DETECT_OVERFLOW_SUB(tmp_a - carry, tmp_b);
    }
    if (!c->number[c->size - 1] && c->size > 1)
        bign_resize(c, c->size - 1);
}

static inline void bign_mul(bign *a, bign *b, bign *c)
{
    bign *a_tmp = bign_alloc(1);
    bign_cpy(a, a_tmp);

    if ((!a->number[0] && (a->size == 1)) ||
        (!b->number[0] && (b->size == 1))) {
        /* prevent a == c or b == c */
        bign_resize(c, 1);
        c->number[0] = 0;
        // printk("0 appear");
        return;
    }

    bign_resize(c, 1);
    c->number[0] = 0;

    /* then resize to limit range of a * b */
    bign_resize(c, a->size + b->size);
    for (int i = 0; i < b->size; i++) {
        if (!b->number[i])
            continue;
        unsigned int clz = __fls(b->number[i]) + 1;
        unsigned int shift_cnt = 32 - clz;
        for (unsigned int d = 1U; clz; d <<= 1) {
            if (!!(d & b->number[i])) {
                bign_add(c, a_tmp, c);
            }
            bign_shl(a_tmp, 1);
            clz--;
        }
        bign_shl(a_tmp, shift_cnt);
    }
    int c_size = 0;
    for (c_size = c->size - 1; !c->number[c_size];)
        c_size--;
    if (c->size > 1)
        bign_resize(c, c_size + 1);
}

static inline void bign_shl(bign *a, const int bits)
{
    if (__builtin_clz(a->number[(a->size) - 1]) < bits)
        bign_resize(a, a->size + 1);
    // for (int j = 0; j < bits; j++) {
    //     for (int i = a->size - 2; i >= 0; i--) {
    //         if (!a->number[i])
    //             continue;
    //         if (!!(a->number[i] & 0x80000000))
    //             a->number[i + 1] += 1;
    //         a->number[i] <<= 1;
    //     }
    // }
    // if (!a->number[a->size - 1] && a->size > 1)
    //     bign_resize(a, a->size - 1);
    for (int i = a->size - 1; i > 0; i--)
        a->number[i] = a->number[i] << bits | a->number[i - 1] >> (32 - bits);
    a->number[0] <<= bits;
}

static inline void bign_resize(bign *a, size_t size)
{
    if (a->size == size)
        return;
    a->number = krealloc(a->number, sizeof(int) * size, GFP_KERNEL);
    if (!a->number)
        return;
    if (a->size < size) {
        memset(a->number + a->size, 0, sizeof(int) * (size - a->size));
    }
    a->size = size;
}

static inline void bign_cpy(bign *a, bign *b)
{
    if (a->size > b->size)
        bign_resize(b, a->size);
    memcpy(b->number, a->number, a->size * sizeof(int));
    b->sign = a->sign;
}

static inline void bign_swap(bign *a, bign *b)
{
    bign tmp = *a;
    *a = *b;
    *b = tmp;
}

static inline void bign_free(bign *a)
{
    if (!a)
        return;
    if (a->number)
        kfree(a->number);
    kfree(a);
}

static inline void bign_fib(bign *dest, int fn)
{
    bign_resize(dest, 1);
    if (fn < 2) {
        dest->number[0] = fn;
        return;
    }

    bign *a = bign_alloc(1);
    bign *b = bign_alloc(1);

    a->number[0] = 0;
    b->number[0] = 1;

    for (int i = 2; i <= fn; i++) {
        bign_add(a, b, dest);
        bign_cpy(dest, a);
        bign_swap(a, b);
    }

    bign_free(a);
    bign_free(b);
}

static inline void bign_fib_fast(bign *dest, const int fn)
{
    bign_resize(dest, 1);
    if (fn < 2) {
        dest->number[0] = fn;
        return;
    }

    bign *a = bign_alloc(1);
    bign *b = bign_alloc(1);
    bign *tmp = bign_alloc(1);

    a->number[0] = 0;
    b->number[0] = 1;

    int fn_fls = __fls(fn);
    for (unsigned int d = 1U << fn_fls; d > 0; d >>= 1) {
        /* F(2k) */
        bign_cpy(b, dest);
        bign_shl(dest, 1);
        bign_sub(dest, a, dest);
        bign_mul(dest, a, dest);
        /* F(2k + 1) */
        bign_mul(a, a, tmp);
        bign_mul(b, b, a);
        bign_add(a, tmp, tmp);
        bign_cpy(dest, a);
        bign_cpy(tmp, b);

        if (!!(d & fn)) {
            bign_add(a, b, a);
            bign_swap(a, b);
            bign_cpy(a, dest);
            // printk("bit 1 string a = %s, string b = %s, string dest =
            // %s",bign_to_string(a), bign_to_string(b), bign_to_string(dest));
        }
        // printk("string dest = %s", bign_to_string(dest));
    }

    bign_free(a);
    bign_free(b);
    bign_free(tmp);
}