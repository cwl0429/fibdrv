
#include "bignum.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define DETECT_OVERFLOW(x, y) ((x) > (~y) ? 1 : 0)
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
        // printk("tmp_a = %ud, tmp_b = %ud", tmp_a, tmp_b);
        c->number[i] = tmp_a + tmp_b + carry;
        // printk("loop %ud done sum = %ud", i, c->number[i]);
        carry = DETECT_OVERFLOW(tmp_a, tmp_b);
    }
    // for (int i = 0; i < c->size; i++)
    //     printk("number %d = %x", i, c->number[i]);
    if (!c->number[c->size - 1] && c->size > 1)
        resize_BigN(c, c->size - 1);
}

static inline void mul_BigN(BigN *a, BigN *b, BigN *c)
{
    printk("\nstart mul");
    resize_BigN(c, a->size + b->size);
    BigN *a_tmp = alloc_BigN(1);
    cpy_BigN(a, a_tmp);
    for (int i = 0; i < b->size; i++) {
        printk("i = %d", i);
        for (unsigned int d = 1U; d; d <<= 1) {
            printk("d = %ud, before string c = %s", d, to_string_BigN(c));
            if (!!(d & b->number[i])) {
                printk("enter add");
                add_BigN(c, a_tmp, c);
            }
            shl_BigN(a_tmp, 1);
            printk("d = %ud, after string c = %s", d, to_string_BigN(c));
        }
    }


    int c_size = 0;
    for (c_size = c->size - 1; !c->number[c_size];)
        c_size--;
    if (c->size > 1)
        resize_BigN(c, c_size + 1);
    printk("BigN c = %s", to_string_BigN(c));
}

static inline void shl_BigN(BigN *a, const int bits)
{
    resize_BigN(a, a->size + 1);
    for (int j = 0; j < bits; j++) {
        for (int i = a->size - 2; i >= 0; i--) {
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

static inline void fib_BigN(BigN *dest, int fn)
{
    printk("_________");
    printk("start fib %d", fn);
    resize_BigN(dest, 1);
    if (fn < 2) {
        dest->number[0] = fn;
        return;
    }

    BigN *a = alloc_BigN(1);
    BigN *b = alloc_BigN(1);

    a->number[0] = 0;
    b->number[0] = 1;

    for (int i = 2; i <= fn; i++) {
        add_BigN(a, b, dest);
        cpy_BigN(dest, a);
        swap_BigN(a, b);
    }

    free_BigN(a);
    free_BigN(b);
}