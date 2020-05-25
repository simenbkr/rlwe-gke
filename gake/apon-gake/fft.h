#ifndef FFT_H
#define FFT_H

#include <stdint.h>
#include "poly.h"

void convolution(const uint64_t *a, const uint64_t *b, uint64_t *res);

#endif //FFT_H
