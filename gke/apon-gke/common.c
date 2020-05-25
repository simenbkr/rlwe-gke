#include "common.h"

void create_player(int id, player *a, poly *public_point, int weird) {
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

    if (weird) {
        gen_poly(&a->ep, SIGMA_2);
    } else {
        gen_poly(&a->ep, SIGMA_1);
    }

    gen_poly(&a->epp, SIGMA_1);

    poly *tmp = malloc(sizeof(poly));
    zero_out(tmp);
    poly_mult_mod(&a->secret, public_point, tmp);
    poly_add(tmp, &a->e, &a->z);
    free(tmp);
}

void create_empty_player(player *a, int id) {
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


uint32_t modq(int32_t x) {
    //return mod((uint32_t) x, APON_Q);
    return modq_64(x);
}

uint64_t modq_64(int64_t x) {

    return x >= APON_Q ? x % APON_Q : x;

    /*if(x < APON_Q && x >= 0) {
        return x;
    }

    if(x < 0) {
        return modq_64(-x);
    }

    return x - ((25U * x) >> 40) * APON_Q;*/

    /*uint64_t c = UINT64_C(0xFFFFFFFFFFFFFFFF) / APON_Q + 1;
    uint64_t lowbits = c * x;

    printf("m(%lu) == %lu and ", x, (((__uint128_t)lowbits * APON_Q) >> 64));
    return (((__uint128_t)lowbits * APON_Q) >> 64);*/

    uint64_t ret = x % APON_Q;
    printf("m(%lu) == %lu and ", x, ret);
    return ret;


    /*int64_t tmp_a = (int64_t) x;
    if (tmp_a < 0) {
        int64_t t = tmp_a % APON_Q;
        if (t > -APON_Q) {
            //printf("m(%lu) == %lu and ", x, t + APON_Q);
            return (t + APON_Q);
        } else {

            return (tmp_a % APON_Q);
        }
    } else {
        printf("m(%lu) == %lu and ", x, tmp_a % APON_Q);
        return (tmp_a % APON_Q);
    }*/
}

uint32_t mod(uint32_t a, uint64_t m) {
    int32_t tmp_a, tmp_m;
    tmp_a = (int32_t) a;
    tmp_m = (int32_t) m;

    if (tmp_a < 0) {
        int32_t t = tmp_a % tmp_m;
        if (t > -m) {
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

    if (tmp_a < 0) {
        int64_t t = tmp_a % tmp_m;
        if (t > -m) {
            return (uint64_t) (t + tmp_m);
        } else {
            return (uint64_t) (tmp_a % tmp_m);
        }
    } else {
        return (uint64_t) (tmp_a % tmp_m);
    }
}