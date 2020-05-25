#ifndef EDUCTION_H
#define REDUCTION_H
#include <stdint.h>

#define u128 __uint128_t

uint64_t barrett_128(u128 a);
uint64_t barrett_64(uint64_t a);
uint64_t montgomery(u128 a);

#endif //REDUCTION_H
