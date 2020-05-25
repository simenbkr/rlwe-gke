#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpucycles.h"
#include "randombytes.h"
#include "monocypher.h"

typedef struct {
    uint8_t id;
    uint8_t public_key[32];
    uint8_t secret_key[32];
    uint8_t i;
    uint8_t **session_keys;
} player_t;

void create_player(player_t *player, int n, int id) {
    player->id = id;
    _randombytes(player->secret_key, 32);
    crypto_x25519_public_key(player->public_key, player->secret_key);
    player->session_keys = malloc(n * 32 * sizeof(uint8_t));
    for (int i = 0; i < n; ++i) {
        player->session_keys[i] = malloc(32);
    }
    player->i = 0;
}

void clean_player(player_t *player) {
    free(player->session_keys);
    free(player);
}

void key_exchange(player_t *init, player_t *resp) {
    uint8_t session_key[32];
    crypto_key_exchange(session_key, init->secret_key, resp->public_key);
    memcpy(init->session_keys[init->i++], session_key, 32);
}

int test_n_players(int n, uint64_t *clock_cycles, int cycle_index) {

    // Generate all players
    player_t **playerlist;
    playerlist = malloc(sizeof(player_t) * n);
    for (int i = 0; i < n; ++i) {
        player_t *k = malloc(sizeof(player_t));
        //create_player(k, n, i);
        playerlist[i] = k;
    }

    long long start, end;
    start = count_cycles();
    for (int i = 0; i < n; ++i) {
        create_player(playerlist[i], n, i);
    }


    for (int i = 0; i < n; ++i) {
        player_t *init = playerlist[i];
        for (int j = 0; j < n; ++j) {
            player_t *resp = playerlist[j];
            if(init->id == resp->id) {
                continue;
            }
            key_exchange(init, resp);
        }
    }

    end = count_cycles();
    clock_cycles[cycle_index] = end - start;

    // Clean up memory
    for (int i = 0; i < n; ++i) {
        clean_player(playerlist[i]);
    }

    free(playerlist);

    return 0;
}


int main() {

    int player_number = 3;
    int player_max = 20;
    int iterations = 1000;
    int i;
    int failures;
    long long cycles = 0;

    clock_t start, end;

    for (int c = player_number; c <= player_max; c++) {
        double sum = 0;
        failures = 0;
        uint64_t clock_cycles[iterations];
        for (i = 0; i < iterations; i++) {

            start = clock();
            long long start_c = count_cycles();
            /*if (test_n_players(c) > 0) {
                failures++;
            }*/
            test_n_players(c, clock_cycles, i);
            long long end_c = count_cycles();
            cycles += (end_c - start_c);
            end = clock();
            sum += (double) (end - start) / CLOCKS_PER_SEC;
        }

        char filename[10];
        sprintf(filename, "%d.txt", c);
        FILE *fp = fopen(filename, "w");
        for (int j = 0; j < iterations; ++j) {
            fprintf(fp, "%lu,", clock_cycles[j]);
        }
        printf("Wrote to %s\n", filename);

        double avg_time = sum / (double) iterations;
        double failure_rate = (double) failures / (double) iterations;
        printf("PARTIES: %d.\nThere was %f percentage failures.\nTotal: %f. Avg %fs on average.\nCycles: %lld, COV: %f\n\n",
               c, failure_rate,
               sum, avg_time, cycles, (float) cycles / (float) (iterations * (player_max - player_number + 1)));

    }

    return 0;

}