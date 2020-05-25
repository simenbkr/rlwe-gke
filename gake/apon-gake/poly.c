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
        /*a->coefficients[i] = modq_64((uint64_t) ((uint32_t) r[3 * i] << 24u
                                                 | (uint32_t) r[3 * i + 1] << 16u
                                                 | (uint32_t) r[3 * i + 2] << 8u
                                                 | (uint32_t) r[3 * i + 3]));*/

        a->coefficients[i] = modq_64((uint64_t) ((uint64_t) r[8 * i + 0] << 56u
                                                 | (uint64_t) r[8 * i + 1] << 48u
                                                 | (uint64_t) r[8 * i + 2] << 40u
                                                 | (uint64_t) r[8 * i + 3] << 32u
                                                 | (uint64_t) r[8 * i + 4] << 24u
                                                 | (uint64_t) r[8 * i + 5] << 16u
                                                 | (uint64_t) r[8 * i + 6] << 8u
                                                 | (uint64_t) r[8 * i + 7]));

        //a->coefficients[i] = modq_64(a->coefficients[i]);
    }

    /*while (!a->coefficients[0]) {
        a->coefficients[0] = random_u64();
        a->coefficients[0] = modq_64(a->coefficients[0]);
    }*/
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

unsigned char *poly_encode(poly *a) {

    unsigned char *enc;
    enc = malloc(sizeof(unsigned char) * APON_N * 4);
    bzero(enc, sizeof(unsigned char) * APON_N * 4);

    if (!enc) {
        printf("Yikes");
        exit(1);
    }

    uint32_t tmp1;
    for (int i = 0; i < APON_N; ++i) {

        tmp1 = mod(a->coefficients[i], APON_Q);

        enc[4 * i] = (unsigned char) ((tmp1 >> 24) & 0xff);
        enc[4 * i + 1] = (unsigned char) ((tmp1 >> 16) & 0xff);
        enc[4 * i + 2] = (unsigned char) ((tmp1 >> 8) & 0xff);
        enc[4 * i + 3] = (unsigned char) ((tmp1) & 0xff);
    }

    return enc;

}

void poly_decode(const unsigned char *enc, poly *r) {
    zero_out(r);
    for (int i = 0; i < APON_N; ++i) {

        uint32_t tmp = (uint32_t) enc[4 * i] << 24
                       | (uint32_t) enc[4 * i + 1] << 16
                       | (uint32_t) enc[4 * i + 2] << 8
                       | (uint32_t) enc[4 * i + 3];

        r->coefficients[i] = mod(tmp, APON_Q);
    }
}

void poly_mult_mod(poly *a, poly *b, poly *r) {
    convolution(a->coefficients, b->coefficients, r->coefficients);
}

void poly_to_uint8(const poly *a, uint8_t *out) {
    for (int i = 0; i < APON_N; ++i) {
        out[8 * i + 0] = (a->coefficients[i] >> 56) & 0xff;
        out[8 * i + 1] = (a->coefficients[i] >> 48) & 0xff;
        out[8 * i + 2] = (a->coefficients[i] >> 40) & 0xff;
        out[8 * i + 3] = (a->coefficients[i] >> 32) & 0xff;
        out[8 * i + 4] = (a->coefficients[i] >> 24) & 0xff;
        out[8 * i + 5] = (a->coefficients[i] >> 16) & 0xff;
        out[8 * i + 6] = (a->coefficients[i] >> 8) & 0xff;
        out[8 * i + 7] = (a->coefficients[i] >> 0) & 0xff;

    }
}

void uint8_to_poly(const uint8_t *in, poly *a) {
    for (int i = 0; i < APON_N; ++i) {
        a->coefficients[i] = (((uint64_t)in[8 * i + 0]) << 56)
                           | (((uint64_t)in[8 * i + 1]) << 48)
                           | (((uint64_t)in[8 * i + 2]) << 40)
                           | (((uint64_t)in[8 * i + 3]) << 32)
                           | (((uint64_t)in[8 * i + 4]) << 24)
                           | (((uint64_t)in[8 * i + 5]) << 16)
                           | (((uint64_t)in[8 * i + 6]) <<  8)
                           | (((uint64_t)in[8 * i + 7]) <<  0);
    }
}