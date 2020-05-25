#include <stdint.h>
#include "kex.h"
#include "constant_time_tricks.h"

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


void rec_msg(poly *b, poly *rec) {
    /**
     * Given a ring element b, generate a reconciliation polynomial.
     */

    zero_out(rec);

    uint64_t cond0 = GKE_Q / 2;
    uint64_t cond1 = GKE_Q;
    uint64_t cond2 = 3 * GKE_Q / 2;
    uint64_t cond3 = 2 * GKE_Q;

    for (int i = 0; i < GKE_N; ++i) {
        uint64_t random = random_u64();

        uint64_t t = dbl(b->coefficients[i], random);

        if ((t >= cond0 && t <= cond1) ||
            (t >= cond2 && t <= cond3)) {
            rec->coefficients[i] = 1;
        } else {
            rec->coefficients[i] = 0;
        }
    }
    //print_poly(rec);
}

void rec_key_ct(const poly *rec, const poly *b, poly *key) {

    /*zero_out(key);
    uint64_t new_q = GKE_Q * (uint64_t) 2;
    uint64_t one_eight = new_q / (uint64_t) 8;

    uint64_t cond1 = (uint64_t) 3 * one_eight + (uint64_t) 1; // 34132525057
    uint64_t cond2 = (uint64_t) 7 * one_eight + (uint64_t) 1; // 79642558465
    uint64_t cond3 = (uint64_t) one_eight;                    // 11377508352
    uint64_t cond4 = (uint64_t) 5 * one_eight + (uint64_t) 1; // 56887541761
    */

    for (int i = 0; i < GKE_N; ++i) {
        uint64_t t = b->coefficients[i] << (uint64_t) 1;
        key->coefficients[i] = (ct_eq_u64(rec->coefficients[i], 0) & ct_ge_u64(t, 34132525057) & ct_le_u64(t, 79642558465)) |
                               (ct_eq_u64(rec->coefficients[i], 1) & ct_ge_u64(t, 11377508352) & ct_le_u64(t, 56887541761));
    }
}

void rec_msg_ct(const poly *b, poly *rec) {
    zero_out(rec);

    /*uint64_t cond0 = GKE_Q / (uint64_t) 2; // 22755016704
    uint64_t cond1 = GKE_Q; // 45510033409
    uint64_t cond2 = (uint64_t) 3 * GKE_Q / (uint64_t) 2; // 68265050113
    uint64_t cond3 = (uint64_t) 2 * GKE_Q; // 91020066818*/

    for (int i = 0; i < 16; ++i) {
        uint64_t r = random_u64();
        for (int j = 0; j < 64; ++j) {
            //printf("%d\n", i*64 + j);
            uint64_t t = dbl(b->coefficients[i * 64 + j], r);
            rec->coefficients[i * 64 + j] = (ct_ge_u64(t, 22755016704) & ct_le_u64(t, 45510033409)) | (ct_ge_u64(t, 68265050113) & ct_le_u64(t, 91020066818));
            r >>= 1;
        }
    }
}




void rec_key(const poly *rec, const poly *b, poly *key) {
    /**
     * Given a ring element and the reconciliation vector, generate the shared key.
     */

    zero_out(key);
    uint64_t new_q = GKE_Q * 2;
    uint64_t one_eight = new_q / 8;

    uint64_t cond1 = 3 * one_eight + 1;
    uint64_t cond2 = 7 * one_eight + 2;
    uint64_t cond3 = one_eight;
    uint64_t cond4 = 5 * one_eight + 1;


    for (int i = 0; i < GKE_N; i++) {
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

    for (int i = 0; i < GKE_N; ++i) {
        if(a->coefficients[i] >= (GKE_Q / 4) && a->coefficients[i] <= (3*GKE_Q / 4)) {
            b->coefficients[i] = 1;
        }
    }

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

