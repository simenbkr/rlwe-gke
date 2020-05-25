#include "common.h"
#include <oqs/oqs.h>

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

    //OQS_STATUS status = OQS_SIG_falcon_512_keypair(a->falcon_pub, a->falcon_priv);

    //if(status != OQS_SUCCESS) {
    //    printf("Unable to create falcon_512 keypair!\n");
    //}

}

void init_signature_keys(player *a) {
    OQS_STATUS status = OQS_SIG_falcon_512_keypair(a->falcon_pub, a->falcon_priv);
    if(status != OQS_SUCCESS) {
        printf("Unable to create falcon_512 keypair!\n");
    }
}

void create_raw_from_poly_msg(const poly_message_t *msg, uint8_t *buffer, uint32_t nonce) {

    buffer[0] = (msg->message_id >> 8) & 0xff;
    buffer[1] = (msg->message_id >> 0) & 0xff;

    memcpy(buffer + 2, msg->message, LENGTH_PACKED);

    buffer[LENGTH_PACKED + 2] = (nonce >> 24) & 0xff;
    buffer[LENGTH_PACKED + 3] = (nonce >> 16) & 0xff;
    buffer[LENGTH_PACKED + 4] = (nonce >>  8) & 0xff;
    buffer[LENGTH_PACKED + 5] = (nonce >>  0) & 0xff;
}

void create_raw_from_rec_msg(const rec_message_t *msg, uint8_t *buffer, uint32_t nonce) {

    buffer[0] = (msg->message_id >> 8) & 0xff;
    buffer[1] = (msg->message_id >> 0) & 0xff;

    memcpy(buffer + 2, msg->message, 128);

    buffer[128 + 3] = (nonce >> 24) & 0xff;
    buffer[128 + 4] = (nonce >> 16) & 0xff;
    buffer[128 + 5] = (nonce >>  8) & 0xff;
    buffer[128 + 6] = (nonce >>  0) & 0xff;
}


void create_poly_message_from_round(const player *p, int round, poly_message_t *msg) {

    size_t sig_len;
    msg->player_id = p->id;
    uint8_t raw[LENGTH_PACKED + 2 + 4];
    create_raw_from_poly_msg(msg, raw, p->nonce);
    switch(round) {
        default:
        case 1:
            msg->message_id = 1;
            poly_to_uint8(&p->z, msg->message);
            break;
        case 2:
            msg->message_id = 2;
            poly_to_uint8(&p->X, msg->message);
            break;
    }
    create_raw_from_poly_msg(msg, raw, p->nonce);
    OQS_STATUS status = OQS_SIG_falcon_512_sign(msg->sign, &sig_len, raw, LENGTH_PACKED + 2 + 4, p->falcon_priv);

    if(status != OQS_SUCCESS) {
        printf("Could not sign the message.\n");
    }

    msg->sign_len = (uint16_t) sig_len;
}

void create_rec_message(const player *p, const poly *rec, rec_message_t *msg) {
    size_t sig_len;
    msg->player_id = p->id;
    msg->message_id = 3;

    for (int i = 0; i < 128; ++i) {
        msg->message[i] = (rec->coefficients[8 * i + 0] << 7)
                          | (rec->coefficients[8 * i + 1] << 6)
                          | (rec->coefficients[8 * i + 2] << 5)
                          | (rec->coefficients[8 * i + 3] << 4)
                          | (rec->coefficients[8 * i + 4] << 3)
                          | (rec->coefficients[8 * i + 5] << 2)
                          | (rec->coefficients[8 * i + 6] << 1)
                          | (rec->coefficients[8 * i + 7] << 0);
    }

    uint8_t raw[134];
    create_raw_from_rec_msg(msg, raw, p->nonce);
    OQS_SIG_falcon_512_sign(msg->sign, &sig_len, raw, 128 + 2 + 4, p->falcon_priv);
    msg->sign_len = sig_len;
}


int verify_poly_message(const player *sender, const poly_message_t *msg) {
    uint8_t raw[LENGTH_PACKED + 2 + 4];
    create_raw_from_poly_msg(msg, raw, sender->nonce);

    OQS_STATUS status = OQS_SIG_falcon_512_verify(raw, LENGTH_PACKED + 2 + 4, msg->sign, msg->sign_len, sender->falcon_pub);

    if(status == OQS_SUCCESS) {
        return 1;
    }
    return 0;
}

int verify_rec_message(const player *sender, const rec_message_t *msg) {
    uint8_t raw[134];
    create_raw_from_rec_msg(msg, raw, sender->nonce);
    OQS_STATUS status = OQS_SIG_falcon_512_verify(raw, 128 + 2 +4, msg->sign, msg->sign_len, sender->falcon_pub);

    if(status == OQS_SUCCESS) {
        return 1;
    }

    return 0;
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

uint64_t modq_64(uint64_t x) {
    int64_t tmp_a = (int64_t) x;
    if (tmp_a < 0) {
        int64_t t = tmp_a % APON_Q;
        if (t > -APON_Q) {
            return (t + APON_Q);
        } else {
            return (tmp_a % APON_Q);
        }
    } else {
        return (tmp_a % APON_Q);
    }
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

void poly_message_to_uint8(const poly_message_t *msg, uint8_t *buffer) {

    buffer[0] = (msg->player_id >> 8) & 0xff;
    buffer[1] = (msg->player_id) & 0xff;

    buffer[2] = (msg->message_id >> 8) & 0xff;
    buffer[3] = (msg->message_id) & 0xff;

    buffer[4] = (msg->sign_len >> 8) & 0xff;
    buffer[5] = (msg->sign_len) & 0xff;

    memcpy(buffer + 6, msg->message, LENGTH_PACKED);
    memcpy(buffer + 6 + LENGTH_PACKED, msg->sign, SIGN_LENGTH);
}

void uint8_to_poly_msg(const uint8_t *raw, poly_message_t *msg) {

    msg->player_id  = (raw[0] << 8) | (raw[1]);
    msg->message_id = (raw[2] << 8) | (raw[3]);
    msg->sign_len = (raw[4] << 8) | (raw[5]);

    memcpy(msg->message, raw + 6, LENGTH_PACKED);
    memcpy(msg->sign, raw + LENGTH_PACKED + 6, SIGN_LENGTH);
}

void rec_message_to_uint8(const rec_message_t *msg, uint8_t *buffer) {

    buffer[0] = (msg->player_id >> 8) & 0xff;
    buffer[1] = (msg->player_id) & 0xff;

    buffer[2] = (msg->message_id >> 8) & 0xff;
    buffer[3] = (msg->message_id) & 0xff;

    memcpy(buffer + 4, msg->message, 128);
    memcpy(buffer + 4 + 128, msg->sign, SIGN_LENGTH);

}


void uint8_to_rec_message(const uint8_t *raw, rec_message_t *msg) {

    msg->player_id = (raw[0] << 8) | (raw[1]);
    msg->message_id = (raw[2] << 8) | (raw[3]);

    memcpy(msg->message, raw + 4, 128);
    memcpy(msg->sign, raw + 128 + 4, SIGN_LENGTH);
}

void deserialize_chat_msg(const chat_message_t *msg, uint8_t *raw) {
    raw[0] = (msg->player_id >> 8) & 0xff;
    raw[1] = (msg->player_id) & 0xff;

    raw[2] = (msg->msg_len >> 8) & 0xff;
    raw[3] = (msg->msg_len) & 0xff;

    memcpy(raw + 4, (uint8_t*)msg->player_name, 20);
    memcpy(raw + 4 + 20, msg->nonce, 24);
    memcpy(raw + 4 + 20 + 24, msg->mac, 16);
    memcpy(raw + 4 + 20 + 24 + 16, msg->data, 1024);
}

void serialize_chat_msg(const uint8_t *raw, chat_message_t *msg) {

    msg->player_id = (raw[0] << 8) | (raw[1]);
    msg->msg_len   = (raw[2] << 8) | (raw[3]);

    memcpy(msg->player_name, raw + 4, 20);
    memcpy(msg->nonce, raw + 4 + 20, 24);
    memcpy(msg->mac, raw + 4 + 20 + 24, 16);
    memcpy(msg->data, raw + 4 + 20 + 24 + 16, 1024);
}

void serialize_vk_msg(const uint8_t *raw, vk_message_t *msg) {

    msg->player_id = (raw[0] << 8) | raw[1];
    memcpy(msg->verification_key, raw + 2, VK_LENGTH);
}

void deserialize_vk_msg(const vk_message_t *msg, uint8_t *raw) {

    raw[0] = (msg->player_id >> 8) & 0xff;
    raw[1] = (msg->player_id) & 0xff;

    memcpy(raw + 2, msg->verification_key, VK_LENGTH);
}