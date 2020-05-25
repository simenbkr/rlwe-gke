#ifndef FFT_H
#define FFT_H

#include <stdint.h>

void convolution(const uint64_t *a, const uint64_t *b, uint64_t *res);
void ntt(const uint64_t *roots, uint64_t *x);
void invntt(const uint64_t *iroots, uint64_t *x);
#endif //FFT_H
