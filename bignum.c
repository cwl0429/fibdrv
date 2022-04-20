
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


static inline BigN *alloc_BigN(size_t size)
{
    BigN *bn = kmalloc(sizeof(BigN), GFP_KERNEL);
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

static inline char *to_string_BigN(BigN *a)
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

static inline void add_BigN(BigN *a, BigN *b, BigN *c)
{
    resize_BigN(c, a->size + 1);
    unsigned int carry = 0;
    for (int i = 0; i < c->size; i++) {
        unsigned int tmp_a = (i < a->size) ? a->number[i] : 0;
        unsigned int tmp_b = (i < b->size) ? b->number[i] : 0;
        c->number[i] = tmp_a + tmp_b + carry;
        carry = DETECT_OVERFLOW(tmp_a, tmp_b + carry);
    }
    if (!c->number[c->size - 1] && c->size > 1)
        resize_BigN(c, c->size - 1);
}

static inline void sub_BigN(BigN *a, BigN *b, BigN *c)
{
    resize_BigN(c, a->size);
    unsigned int carry = 0;
    for (int i = 0; i < c->size; i++) {
        long tmp_a = (i < a->size) ? (long) a->number[i] : 0;
        long tmp_b = (i < b->size) ? (long) b->number[i] : 0;
        c->number[i] = tmp_a - tmp_b - carry;
        carry = DETECT_OVERFLOW_SUB(tmp_a - carry, tmp_b);
    }
    if (!c->number[c->size - 1] && c->size > 1)
        resize_BigN(c, c->size - 1);
    // for (int i = 0; i < c->size; i++)
    //     printk("number %d = %x", i, c->number[i]);
}

static inline void mul_BigN(BigN *a, BigN *b, BigN *c)
{
    // printk("enter mul");
    BigN *a_tmp = alloc_BigN(1);
    cpy_BigN(a, a_tmp);

    if ((!a->number[0] && (a->size == 1)) ||
        (!b->number[0] && (b->size == 1))) {
        /* prevent a == c or b == c */
        resize_BigN(c, 1);
        c->number[0] = 0;
        // printk("0 appear");
        return;
    }

    resize_BigN(c, 1);
    c->number[0] = 0;

    /* then resize to limit range of a * b */
    resize_BigN(c, a->size + b->size);
    int cnt = 0;
    for (int i = 0; i < b->size; i++) {
        if (!b->number[i])
            continue;
        unsigned int clz = __fls(b->number[i]) + 1;
        for (unsigned int d = 1U; clz; d <<= 1) {
            if (!!(d & b->number[i])) {
                // printk("mul string C = %s, string a_tmp =
                // %s",to_string_BigN(c), to_string_BigN(a_tmp));
                add_BigN(c, a_tmp, c);
                // printk("string c = %s",to_string_BigN(c));
            }
            // printk("cnt = %d, d = %u, clz = %u, i = %d, b->size = %d", cnt,
            // d, clz, i, b->size);
            shl_BigN(a_tmp, 1);
            clz--;
        }
        printk("cnt = %d", cnt);
    }
    int c_size = 0;
    for (c_size = c->size - 1; !c->number[c_size];)
        c_size--;
    if (c->size > 1)
        resize_BigN(c, c_size + 1);
}

static inline void shl_BigN(BigN *a, const int bits)
{
    resize_BigN(a, a->size + 1);
    for (int j = 0; j < bits; j++) {
        for (int i = a->size - 2; i >= 0; i--) {
            if (!a->number[i])
                continue;
            if (!!(a->number[i] & 0x80000000))
                a->number[i + 1] += 1;
            a->number[i] <<= 1;
        }
    }
    if (!a->number[a->size - 1] && a->size > 1)
        resize_BigN(a, a->size - 1);
}

static inline void resize_BigN(BigN *a, size_t size)
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

static inline void cpy_BigN(BigN *a, BigN *b)
{
    if (a->size > b->size)
        resize_BigN(b, a->size);
    memcpy(b->number, a->number, a->size * sizeof(int));
    b->sign = a->sign;
}

static inline void swap_BigN(BigN *a, BigN *b)
{
    BigN tmp = *a;
    *a = *b;
    *b = tmp;
}

static inline void free_BigN(BigN *a)
{
    if (!a)
        return;
    if (a->number)
        kfree(a->number);
    kfree(a);
}

// static inline void fib_BigN(BigN *dest, int fn)
// {
//     // printk("_________");
//     // printk("start fib %d", fn);
//     resize_BigN(dest, 1);
//     if (fn < 2) {
//         dest->number[0] = fn;
//         return;
//     }

//     BigN *a = alloc_BigN(1);
//     BigN *b = alloc_BigN(1);

//     a->number[0] = 0;
//     b->number[0] = 1;

//     for (int i = 2; i <= fn; i++) {
//         add_BigN(a, b, dest);
//         cpy_BigN(dest, a);
//         swap_BigN(a, b);
//     }

//     free_BigN(a);
//     free_BigN(b);
// }

static inline void fib_fast_BigN(BigN *dest, const int fn)
{
    resize_BigN(dest, 1);
    if (fn < 2) {
        dest->number[0] = fn;
        return;
    }

    BigN *a = alloc_BigN(1);
    BigN *b = alloc_BigN(1);
    BigN *tmp = alloc_BigN(1);

    a->number[0] = 0;
    b->number[0] = 1;

    int fn_fls = __fls(fn);
    for (unsigned int d = 1U << fn_fls; d > 0; d >>= 1) {
        /* F(2k) */
        // printk("\ninit string a = %s, string b = %s, string c =
        // %s",to_string_BigN(a), to_string_BigN(b), to_string_BigN(dest));
        cpy_BigN(b, dest);
        shl_BigN(dest, 1);
        // printk("f(2k) string a = %s, string b = %s, string c =
        // %s",to_string_BigN(a), to_string_BigN(b), to_string_BigN(dest));
        sub_BigN(dest, a, dest);
        // printk("f(2k) string a = %s, string b = %s, string c =
        // %s",to_string_BigN(a), to_string_BigN(b), to_string_BigN(dest));
        mul_BigN(dest, a, dest);
        // printk("f(2k) string a = %s, string b = %s, string c =
        // %s",to_string_BigN(a), to_string_BigN(b), to_string_BigN(dest));
        /* F(2k + 1) */
        mul_BigN(a, a, tmp);
        mul_BigN(b, b, a);
        add_BigN(a, tmp, tmp);
        // printk("f(2k+1) string a = %s, string b = %s, string c =
        // %s",to_string_BigN(a), to_string_BigN(b), to_string_BigN(dest));
        cpy_BigN(dest, a);
        cpy_BigN(tmp, b);
        // printk("cpy string a = %s, string b = %s, string c =
        // %s",to_string_BigN(a), to_string_BigN(b), to_string_BigN(dest));
        if (!!(d & fn)) {
            add_BigN(a, b, a);
            swap_BigN(a, b);
            cpy_BigN(a, dest);
            // printk("bit 1 string a = %s, string b = %s, string c =
            // %s",to_string_BigN(a), to_string_BigN(b), to_string_BigN(dest));
        }
        printk("string dest = %s", to_string_BigN(dest));
    }
    free_BigN(a);
    free_BigN(b);
    free_BigN(tmp);
}