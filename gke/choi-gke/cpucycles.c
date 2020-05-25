#include "cpucycles.h"


long long count_cycles() {
    unsigned long long result;
    asm volatile("rdtsc;"
                 "shlq $32, %%rdx;"
                 "orq %%rdx, %%rax"
    :  "=a" (result)
    :
    : "%rdx");

    return result;
}