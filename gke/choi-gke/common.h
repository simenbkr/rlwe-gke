#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include "params.h"
#include "poly.h"

typedef struct {
    int id;
    poly secret;
    poly z;
    poly e;
    poly ep;
    poly epp;
    poly X;
    poly b;
    poly key;
    poly *Y;
} player;


struct playerlist_t {
    player *player;
    struct playerlist_t *next;
};

typedef struct playerlist_t playerlist_t;

void create_player(int id, player *a, poly *public_point, int weird, int participants);

void create_empty_player(player* a, int id);

uint64_t modq_64(int64_t x);

uint32_t mod(uint32_t a, uint32_t m);

uint64_t uint64_mod(uint64_t a, uint64_t m);

int equal_secrets(player **players, int n);

#endif //COMMON_H
