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

#define REC_MSG_LENGTH 824//2 + 2 + 2 + 128 + 690

typedef struct {
    uint16_t player_id;
    char player_name[20];
    uint16_t msg_len;
    uint8_t nonce[24];
    uint8_t mac[16];
    uint8_t data[1024];
} chat_message_t;

#define CHAT_MESSAGE_LENGTH 1088

typedef struct {
    uint16_t player_id;
    uint8_t message[1024];
} pt_chat_message_t;

#define PT_CHAT_MESSAGE_LENGTH 1026

void serialize_chat_msg(const uint8_t *raw, chat_message_t *msg);

void deserialize_chat_msg(const chat_message_t *msg, uint8_t *raw);

typedef struct {
    uint16_t player_id;
    uint8_t verification_key[OQS_SIG_falcon_512_length_public_key];
} vk_message_t;

#define VK_MESSAGE_LENGTH 2 + OQS_SIG_falcon_512_length_public_key

void serialize_vk_msg(const uint8_t *raw, vk_message_t *msg);

void deserialize_vk_msg(const vk_message_t *msg, uint8_t *raw);

typedef struct {
    char name[20];
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
} player;


struct playerlist_t {
    player *player;
    struct playerlist_t *next;
};

typedef struct playerlist_t playerlist_t;

void create_player(int id, player *a, poly *public_point, int weird);

void init_signature_keys(player *a);

void create_poly_message_from_round(const player *p, int round, poly_message_t *msg);

void create_rec_message(const player *p, const poly *rec, rec_message_t *msg);

int verify_poly_message(const player *sender, const poly_message_t *msg);

int verify_rec_message(const player *sender, const rec_message_t *msg);

void create_empty_player(player *a, int id);

void poly_message_to_uint8(const poly_message_t *msg, uint8_t *buffer);

void uint8_to_poly_msg(const uint8_t *raw, poly_message_t *msg);

void rec_message_to_uint8(const rec_message_t *msg, uint8_t *buffer);

void uint8_to_rec_message(const uint8_t *raw, rec_message_t *msg);


uint32_t modq(int32_t x);

uint64_t modq_64(uint64_t x);

uint32_t mod(uint32_t a, uint64_t m);

uint64_t uint64_mod(uint64_t a, uint64_t m);

#endif //COMMON_H
