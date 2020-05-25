#ifndef KEX_H
#define KEX_H

#include "poly.h"

void rec_msg(poly *b, poly *rec);
void rec_key(const poly *rec, const poly *b, poly *key);
void calculate_x(poly *z_right, poly *z_left, poly *secret, poly *error, poly *X);
//void calculate_x_from_playerlist(player **playerlist, int list_length , player *self);
//void calculate_b_from_playerlist(player **playerlist, int player_count, player *self);
void round2(const poly *a, poly *b);
void rec_msg_ct(const poly *b, poly *rec);
void rec_key_ct(const poly *rec, const poly *b, poly *key);

#endif //KEX_H
