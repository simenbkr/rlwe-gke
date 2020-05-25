#include "common.h"
#include <string.h>

void create_player(int id, player *a, poly *public_point, int weird, int participants) {
    a->id = id;
    zero_out(&a->secret);
    zero_out(&a->z);
    zero_out(&a->e);
    zero_out(&a->ep);
    zero_out(&a->epp);
    zero_out(&a->X);
    zero_out(&a->b);
    zero_out(&a->key);

    gen_poly(&a->secret, SIGMA_1);
    gen_poly(&a->e, SIGMA_1);

    if(weird) {
        gen_poly(&a->ep, SIGMA_2);
    } else {
        gen_poly(&a->ep, SIGMA_1);
    }

    gen_poly(&a->epp, SIGMA_1);

    poly tmp;
    zero_out(&tmp);
    poly_mult_mod(&a->secret, public_point, &tmp);
    poly_add(&tmp, &a->e, &a->z);
    a->Y = malloc(sizeof(poly) * participants);
}

void create_empty_player(player* a, int id) {
    zero_out(&a->secret);
    zero_out(&a->z);
    zero_out(&a->e);
    zero_out(&a->ep);
    zero_out(&a->epp);
    zero_out(&a->X);
    zero_out(&a->b);
    zero_out(&a->key);

    a->id = id;
}

uint64_t modq_64(int64_t x) {
    return x >= GKE_Q ? x % GKE_Q : x;
    //return uint64_mod((uint32_t) x, GKE_Q);
}

uint32_t mod(uint32_t a, uint32_t m) {
    int32_t tmp_a, tmp_m;
    tmp_a = (int32_t) a;
    tmp_m = (int32_t) m;

    if(tmp_a < 0) {
        int32_t t = tmp_a % tmp_m;
        if(t > -m) {
            return (uint32_t) (t + tmp_m);
        } else {
            return (uint32_t) (tmp_a % tmp_m);
        }
    } else {
        return (uint32_t) (tmp_a % tmp_m);
    }
}

uint64_t uint64_mod(uint64_t a, uint64_t m) {

    int64_t tmp_a, tmp_m;
    tmp_a = (int64_t) a;
    tmp_m = (int64_t) m;

    if(tmp_a < 0) {
        int64_t t = tmp_a % tmp_m;
        if(t > -m) {
            return (uint64_t) (t + tmp_m);
        } else {
            return (uint64_t) (tmp_a % tmp_m);
        }
    } else {
        return (uint64_t) (tmp_a % tmp_m);
    }
}

int equal_secrets(player **players, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int check = memcmp(&players[i]->key, &players[j]->key, sizeof(poly));
            if(check) {
                //printf("meep beep\n");
                return 0;
            }
        }
    }

    return 1;
}