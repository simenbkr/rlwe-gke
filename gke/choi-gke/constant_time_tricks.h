#ifndef CONSTANT_TIME_TRICKS_H
#define CONSTANT_TIME_TRICKS_H
#include <stdint.h>

uint64_t ct_isnonzero_u64(uint64_t x);

uint64_t ct_ne_u64(uint64_t x, uint64_t y);

uint64_t ct_eq_u64(uint64_t x, uint64_t y);

uint64_t ct_lt_u64(uint64_t x, uint64_t y);

uint64_t ct_gt_u64(uint64_t x, uint64_t y);

uint64_t ct_le_u64(uint64_t x, uint64_t y);

uint64_t ct_ge_u64(uint64_t x, uint64_t y);

uint64_t ct_mask_u64(uint64_t bit);

uint64_t ct_select_u64(uint64_t x, uint64_t y, uint64_t bit);

static int cmplt_ct(uint64_t *a, uint64_t *b);

#endif //CONSTANT_TIME_TRICKS_H
