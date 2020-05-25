#include "constant_time_tricks.h"


/**
 * These functions are taken from the BCNS implementation of RLWE in TLS.
 * See https://github.com/dstebila/rlwekex.
 */


/* Auxiliary functions for constant-time comparison */

/*
 * Returns 1 if x != 0
 * Returns 0 if x == 0
 * x and y are arbitrary unsigned 64-bit integers
 */
uint64_t ct_isnonzero_u64(uint64_t x) {
    return (x | -x) >> 63;
}

/*
 * Returns 1 if x != y
 * Returns 0 if x == y
 * x and y are arbitrary unsigned 64-bit integers
 */
uint64_t ct_ne_u64(uint64_t x, uint64_t y) {
    return ((x - y) | (y - x)) >> 63;
}

/*
 * Returns 1 if x == y
 * Returns 0 if x != y
 * x and y are arbitrary unsigned 64-bit integers
 */
uint64_t ct_eq_u64(uint64_t x, uint64_t y) {
    return 1 ^ ct_ne_u64(x, y);
}

/* Returns 1 if x < y
 * Returns 0 if x >= y
 * x and y are arbitrary unsigned 64-bit integers
 */
uint64_t ct_lt_u64(uint64_t x, uint64_t y) {
    return (x ^ ((x ^ y) | ((x - y)^y))) >> 63;
}

/*
 * Returns 1 if x > y
 * Returns 0 if x <= y
 * x and y are arbitrary unsigned 64-bit integers
 */
uint64_t ct_gt_u64(uint64_t x, uint64_t y) {
    return ct_lt_u64(y, x);
}

/*
 * Returns 1 if x <= y
 * Returns 0 if x > y
 * x and y are arbitrary unsigned 64-bit integers
 */
uint64_t ct_le_u64(uint64_t x, uint64_t y) {
    return 1 ^ ct_gt_u64(x, y);
}

/*
 * Returns 1 if x >= y
 * Returns 0 if x < y
 * x and y are arbitrary unsigned 64-bit integers
 */
uint64_t ct_ge_u64(uint64_t x, uint64_t y) {
    return 1 ^ ct_lt_u64(x, y);
}

/* Returns 0xFFFF..FFFF if bit != 0
 * Returns            0 if bit == 0
 */
uint64_t ct_mask_u64(uint64_t bit) {
    return 0 - (uint64_t)ct_isnonzero_u64(bit);
}

/* Conditionally return x or y depending on whether bit is set
 * Equivalent to: return bit ? x : y
 * x and y are arbitrary 64-bit unsigned integers
 * bit must be either 0 or 1.
 */
uint64_t ct_select_u64(uint64_t x, uint64_t y, uint64_t bit) {
    uint64_t m = ct_mask_u64(bit);
    return (x & m) | (y & ~m);
}

/* Returns 0 if a >= b
 * Returns 1 if a < b
 * Where a and b are both 3-limb 64-bit integers.
 * This function runs in constant time.
 */
int cmplt_ct(uint64_t *a, uint64_t *b) {
    uint64_t r = 0; /* result */
    uint64_t m = 0; /* mask   */
    int i;
    for (i = 2; i >= 0; --i) {
        r |= ct_lt_u64(a[i], b[i]) & ~m;
        m |= ct_mask_u64(ct_ne_u64(a[i], b[i])); /* stop when a[i] != b[i] */
    }
    return r & 1;
}