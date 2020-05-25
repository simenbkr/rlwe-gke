#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

#include <stdint.h>

void _randombytes(unsigned char *x, unsigned long long xlen);
uint64_t _random_u64();
#endif //RANDOMBYTES_H
