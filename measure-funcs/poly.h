#ifndef POLY_H
#define POLY_H

#include <stdint.h>
#include "params.h"
#include "randombytes.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>



typedef struct {
    uint64_t coefficients[GKE_N];
} poly;

void gen_uniform_poly(poly *a);

void gen_poly(poly *a, uint64_t sigma);

void poly_add(const poly *a, const poly *b, poly *result);

void poly_subtract(poly *a, poly *b, poly *result);

//void poly_mult_mod(poly *a, poly *b, poly *result);

void scalar_multiply(uint64_t, poly *p, poly *result);

void zero_out(poly *p);

void copy_poly(poly *p, poly *new);

void pointwise_mult_mod(poly *a, poly *b, poly *result);

void poly_mod(poly *a, poly *out);

void count_ones(const poly *p);

void print_poly(poly *p);

void print_key_hashed(poly *key);

char* get_val_hashed(poly *val);

unsigned char* poly_encode(poly *a);

void poly_mult_mod(poly *a, poly *b, poly *r);

void poly_decode(const unsigned char *enc, poly *r);

#endif //POLY_H
