#include "poly.h"
#include "common.h"
#include "cpucycles.h"
#include <stdint.h>
#include "fft.h"
#include "math.h"
#include "reduction.h"

#define uint128_t __uint128_t

#include <dgs/dgs.h>

void gen_uniform_poly(poly *a) {

    zero_out(a);
    unsigned char r[APON_N * 8];
    randombytes(r, APON_N * 8);

    for (int i = 0; i < APON_N; i++) {
        a->coefficients[i] = barrett_64((uint64_t) ((uint64_t) r[8 * i + 0] << 56u
                                                 | (uint64_t) r[8 * i + 1] << 48u
                                                 | (uint64_t) r[8 * i + 2] << 40u
                                                 | (uint64_t) r[8 * i + 3] << 32u
                                                 | (uint64_t) r[8 * i + 4] << 24u
                                                 | (uint64_t) r[8 * i + 5] << 16u
                                                 | (uint64_t) r[8 * i + 6] << 8u
                                                 | (uint64_t) r[8 * i + 7]));
    }
}


void gen_poly(poly *a, uint64_t sigma) {
    //TODO make constant time or something idno

    if(sigma > MAX_SAMPLER) {
        zero_out(a);
        uint64_t sigma_squared = SIGMA_2_squared;
        uint64_t increments = 4453033417120800;
        uint64_t sum = 0;
        uint64_t sig = 4453033417120800;

        while (sum != sigma_squared) {
            dgs_disc_gauss_dp_t *D = dgs_disc_gauss_dp_init(sqrt(sig), 0, MAX_WIDTH, DGS_DISC_GAUSS_DEFAULT);

            for (int i = 0; i < APON_N; ++i) {
                int64_t t = D->call(D);
                if(t < 0) {
                    t += APON_Q;
                }
                a->coefficients[i] = barrett_64(barrett_64(t) + a->coefficients[i]);
            }

            sum += sig;
            if (sigma_squared - sum < increments) {
                sig = sigma_squared - sum;
            } else {
                sig = increments;
            }

            if (sum == sigma_squared) {
                break;
            }
        }
    } else {
        dgs_disc_gauss_dp_t *D = dgs_disc_gauss_dp_init(sigma, 0, MAX_WIDTH, DGS_DISC_GAUSS_SIGMA2_LOGTABLE);

        int64_t t;
        for (int i = 0; i < APON_N; ++i) {
            t = D->call(D);
            if(t < 0) {
                t += APON_Q;
            }
            a->coefficients[i] = barrett_64(t);
        }
    }
}


void poly_add(const poly *a, const poly *b, poly *result) {
    for (int i = 0; i < 256; i++) {
        result->coefficients[i      ] = barrett_64(a->coefficients[i      ] + b->coefficients[i      ]);
        result->coefficients[256 + i] = barrett_64(a->coefficients[256 + i] + b->coefficients[256 + i]);
        result->coefficients[512 + i] = barrett_64(a->coefficients[512 + i] + b->coefficients[512 + i]);
        result->coefficients[768 + i] = barrett_64(a->coefficients[768 + i] + b->coefficients[768 + i]);
    }
}


void poly_subtract(poly *a, poly *b, poly *result) {
    for (int i = 0; i < 256; i++) {
        result->coefficients[i      ] = barrett_64(a->coefficients[i      ] - b->coefficients[i      ] + APON_Q * 3);
        result->coefficients[256 + i] = barrett_64(a->coefficients[256 + i] - b->coefficients[256 + i] + APON_Q * 3);
        result->coefficients[512 + i] = barrett_64(a->coefficients[512 + i] - b->coefficients[512 + i] + APON_Q * 3);
        result->coefficients[768 + i] = barrett_64(a->coefficients[768 + i] - b->coefficients[768 + i] + APON_Q * 3);
    }
}

void pointwise_mult_mod(poly *a, poly *b, poly *result) {
    for (int i = 0; i < APON_N; i++) {
        result->coefficients[i] = (a->coefficients[i] * b->coefficients[i]) % APON_Q;
    }
}

uint64_t mulmod(uint64_t a, uint64_t b) {

    if (a == 0 || b == 0) {
        return 0;
    }

    if (a == 1) {
        return b;
    }

    if (b == 1) {
        return a;
    }

    uint128_t tmp_a = (uint128_t) a;
    uint128_t tmp_b = (uint128_t) b;

    // JUST CAST TO 128-BIT INTEGERS LOL
    //return (uint64_t) ((tmp_a * tmp_b) % APON_Q);
    //return montgomery(tmp_a * tmp_b);
    return barrett_128(tmp_a * tmp_b);
}


void scalar_multiply(uint64_t scalar, poly *p, poly *result) {
    for (int i = 0; i < 256; ++i) {
        result->coefficients[i] = mulmod(p->coefficients[i], scalar);
        result->coefficients[256 + i] = mulmod(p->coefficients[256 + i], scalar);
        result->coefficients[512 + i] = mulmod(p->coefficients[512 + i], scalar);
        result->coefficients[768 + i] = mulmod(p->coefficients[768 + i], scalar);
    }
}

void zero_out(poly *p) {
    memset(p, 0, sizeof(poly));
}

void copy_poly(poly *p, poly *new) {
    zero_out(new);
    for (int i = 0; i < APON_N; ++i) {
        new->coefficients[i] = p->coefficients[i];
    }
}

void count_ones(const poly *p) {
    int c = 0;
    for (int i = 0; i < APON_N; ++i) {
        if (p->coefficients[i] == 1) {
            c += 1;
        }
    }
    printf("The given polynomial has %d ones.\n", c);
}

void print_poly(poly *p) {
    for (int i = 0; i < APON_N; i++) {
        printf("%d: %lu | ", i, p->coefficients[i]);
    }
    printf("\n");
}

void print_key_hashed(poly *key) {

    uint8_t out[64];
    sha3(key, APON_N * 4, out, 64);

    for (int i = 0; i < 64; ++i) {
        printf("%02x", out[i]);
    }

    printf("\n");
}

char *get_val_hashed(poly *val) {
    uint8_t out[64];
    char *enc;
    enc = malloc(64 * sizeof(char));
    sha3(val, APON_N, out, 64);

    for (int i = 0; i < 64; i += 2) {
        sprintf(enc, "%02x", out[i]);
    }

    return enc;
}

void poly_mult_mod(poly *a, poly *b, poly *r) {
    convolution(a->coefficients, b->coefficients, r->coefficients);
}