#include <stdint.h>
#include "kem.h"
#include "common.h"
#include "constant_time_tricks.h"
#include "cpucycles.h"

/**
 * Peikert-style reconciliation. Lets go!
 */
uint64_t dbl(uint64_t a, uint32_t e) {
    /**
     * Return 2*a + r, where r is -1, 1 or 1 with probabilitities 0.25, 0.5, 0.25.
     */
    e = (((e >> 1) & 1) - ((int32_t) (e & 1)));
    return (uint64_t) ((((uint64_t) a) << (uint64_t) 1) - e);
}

void rec_msg_ct(const poly *b, poly *rec) {
    zero_out(rec);

    uint64_t cond0 = APON_Q / (uint64_t) 2;
    uint64_t cond1 = APON_Q;
    uint64_t cond2 = (uint64_t) 3 * APON_Q / (uint64_t) 2;
    uint64_t cond3 = (uint64_t) 2 * APON_Q;

    for (int i = 0; i < 16; ++i) {
        uint64_t r = random_u64();
        for (int j = 0; j < 64; ++j) {
            //printf("%d\n", i*64 + j);
            uint64_t t = dbl(b->coefficients[i * 64 + j], r);
            rec->coefficients[i * 64 + j] = (ct_ge_u64(t, cond0) & ct_le_u64(t, cond1)) | (ct_ge_u64(t, cond2) & ct_le_u64(t, cond3));
            r >>= 1;
        }
    }
}


void rec_msg(poly *b, poly *rec) {
    /**
     * Given a ring element b, generate a reconciliation polynomial.
     */

    zero_out(rec);

    uint64_t cond0 = APON_Q / (uint64_t) 2;
    uint64_t cond1 = APON_Q;
    uint64_t cond2 = (uint64_t) 3 * APON_Q / (uint64_t) 2;
    uint64_t cond3 = (uint64_t) 2 * APON_Q;

    for (int i = 0; i < APON_N; ++i) {
        uint64_t random = random_u64();

        uint64_t t = dbl(b->coefficients[i], random);

        if ((t >= cond0 && t <= cond1) ||
            (t >= cond2 && t <= cond3)) {
            rec->coefficients[i] = 1;
        } else {
            rec->coefficients[i] = 0;
        }
    }
}

void rec_key_ct(const poly *rec, const poly *b, poly *key) {

    zero_out(key);
    uint64_t new_q = APON_Q * (uint64_t) 2;
    uint64_t one_eight = new_q / (uint64_t) 8;

    uint64_t cond1 = (uint64_t) 3 * one_eight + (uint64_t) 1;
    uint64_t cond2 = (uint64_t) 7 * one_eight + (uint64_t) 2;
    uint64_t cond3 = (uint64_t) one_eight;
    uint64_t cond4 = (uint64_t) 5 * one_eight + (uint64_t) 1;

    for (int i = 0; i < APON_N; ++i) {
        uint64_t t = b->coefficients[i] << (uint64_t) 1;
        key->coefficients[i] = (ct_eq_u64(rec->coefficients[i], 0) & ct_ge_u64(t, cond1) & ct_le_u64(t, cond2)) |
                (ct_eq_u64(rec->coefficients[i], 1) & ct_ge_u64(t, cond3) & ct_le_u64(t, cond4));
    }
}


void rec_key(const poly *rec, const poly *b, poly *key) {
    /**
     * Given a ring element and the reconciliation vector, generate the shared key.
     */

    zero_out(key);
    uint64_t new_q = APON_Q * (uint64_t) 2;
    uint64_t one_eight = new_q / (uint64_t) 8;

    uint64_t cond1 = (uint64_t) 3 * one_eight + (uint64_t) 1;
    uint64_t cond2 = (uint64_t) 7 * one_eight + (uint64_t) 2;
    uint64_t cond3 = (uint64_t) one_eight;
    uint64_t cond4 = (uint64_t) 5 * one_eight + (uint64_t) 1;


    for (int i = 0; i < APON_N; i++) {
        uint64_t test = (uint64_t) b->coefficients[i] << (uint64_t) 1;

        if (rec->coefficients[i] == 0) {
            if (test >= cond1 && test <= cond2) {
                key->coefficients[i] = 1;
            }
        } else {
            if (test >= cond3 && test <= cond4) {
                key->coefficients[i] = 1;
            }
        }


    }
}

void round2(const poly *a, poly *b) {

    uint64_t cond1 = APON_Q / (uint64_t) 4;
    uint64_t cond2 = (uint64_t) 3 * APON_Q / (uint64_t) 4;

    for (int i = 0; i < APON_N; ++i) {
        if(a->coefficients[i] >= cond1 && a->coefficients[i] <= cond2) {
            b->coefficients[i] = 1;
        }
    }

}

void calculate_x_from_playerlist(player **playerlist, int list_length, player *self) {
    int i, prev_i, next_i;

    for (i = 0; i < list_length; ++i) {
        if (playerlist[i]->id == self->id) {
            break;
        }
    }

    next_i = (i + 1);
    prev_i = (i - 1);

    while (next_i >= list_length) {
        next_i -= list_length;
    }

    while (prev_i < 0) {
        prev_i += list_length;
    }

    calculate_x(&playerlist[next_i]->z, &playerlist[prev_i]->z, &self->secret, &self->ep, &self->X);
}

void calculate_x(poly *z_right, poly *z_left, poly *secret, poly *error, poly *X) {
    poly *tmp = malloc(sizeof(poly));
    zero_out(X);
    zero_out(tmp);
    poly_subtract(z_right, z_left, tmp);
    poly_mult_mod(tmp, secret, X);
    poly_add(X, error, X);
    free(tmp);
}

void calculate_b_from_playerlist(player **playerlist, int player_count, player *self) {
    int index = 0;

    for (int i = 0; i < player_count; ++i) {
        if (playerlist[i]->id == self->id) {
            index = i;
        }
    }

    poly *tmp, *tmp2, *accumulator;
    tmp = malloc(sizeof(poly));
    tmp2 = malloc(sizeof(poly));
    accumulator = malloc(sizeof(poly));
    zero_out(tmp);
    zero_out(accumulator);
    zero_out(&self->b);

    poly_mult_mod(&playerlist[mod(index - 1, player_count)]->z, &self->secret, tmp);
    scalar_multiply(player_count, tmp, tmp);

    int test = 0;
    for (int i = 0; i < player_count; ++i) {
        uint32_t sc = player_count - i - 1;
        uint32_t ind = mod(index + i, player_count);

        if (sc < 1) {
            //sc = 0;
            break;
        }
        zero_out(tmp2);
        scalar_multiply(sc, &playerlist[ind]->X, tmp2);

        poly_add(tmp2, accumulator, accumulator);
        test++;
    }

    poly_add(accumulator, tmp, &self->b);

    free(tmp);
    free(tmp2);
    free(accumulator);

    for (int j = 0; j < APON_N; ++j) {
        self->b.coefficients[j] = modq_64(self->b.coefficients[j]);
    }

    //printf("Player %d used %d X-values.\n", index, test);
}