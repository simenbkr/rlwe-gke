#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include "params.h"
#include "poly.h"
#include <oqs/oqs.h>

typedef struct {
    uint16_t player_id;
    uint16_t message_id;
    uint16_t sign_len;
    uint8_t message[LENGTH_PACKED];
    uint8_t sign[OQS_SIG_falcon_512_length_signature];
} poly_message_t;

#define MESSAGE_LENGTH 8888 //6 + 8192 + 690 length in unsigned chars or uint8.

typedef struct {
    uint16_t player_id;
    uint16_t message_id;
    uint16_t sign_len;
    uint8_t message[128];
    uint8_t sign[OQS_SIG_falcon_512_length_signature];
} rec_message_t;

#define REC_MSG_LENGTH 824//2 + 2 + 128 + 690

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
    uint8_t falcon_pub[OQS_SIG_falcon_512_length_public_key];
    uint8_t falcon_priv[OQS_SIG_falcon_512_length_secret_key];
    uint32_t nonce;
    poly_message_t poly_msg[2];
    rec_message_t rec_msg;
    poly *Y;
} player;


struct playerlist_t {
    player *player;
    struct playerlist_t *next;
};

typedef struct playerlist_t playerlist_t;

void create_player(int id, player *a, poly *public_point, int weird, int participants);

void init_signature_keys(player *a);

void create_poly_message_from_round(const player *p, int round, poly_message_t *msg);

void create_rec_message(const player *p, const poly *rec, rec_message_t *msg);

int verify_poly_message(const player *sender, const poly_message_t *msg);

int verify_rec_message(const player *sender, const rec_message_t *msg);

void create_empty_player(player* a, int id);

uint64_t modq_64(int64_t x);

uint32_t mod(uint32_t a, uint32_t m);

uint64_t uint64_mod(uint64_t a, uint64_t m);

int equal_secrets(player **players, int n);

#endif //COMMON_H
