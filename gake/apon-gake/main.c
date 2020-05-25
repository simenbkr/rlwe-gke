#include <stdio.h>
#include "params.h"
#include "poly.h"
#include "randombytes.h"
#include "kem.h"
#include "randombytes.h"
#include "constant_time_tricks.h"
#include "keccak.h"
#include "common.h"
#include <time.h>
#include <unistd.h>
#include "cpucycles.h"

#include <dgs/dgs.h>
#include <oqs/oqs.h>


int equal_shared_secret(int n, player **player_list) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int check = memcmp(&player_list[i]->key, &player_list[j]->key, sizeof(poly));
            if (check) {
                return 0;
            }
        }
    }

    return 1;
}

int test_n_players(int n, uint64_t *clock_cycles, int clock_index) {

    int32_t i;
    poly public, rec;
    zero_out(&public);
    zero_out(&rec);
    gen_uniform_poly(&public);

    player **playerlist;
    playerlist = malloc(sizeof(player) * n);

    /**
     * Round 1: P_i samples s_i, e_i from Normal dist, broadcasts z_i = a * s_i + e_i
     */

    for (i = 0; i < n; ++i) {
        int weird = 0;
        if (i == 0) {
            weird = 1;
        }

        player *k;
        k = malloc(sizeof(player));
        //create_player(i, k, &public, weird);
        playerlist[i] = k;
        init_signature_keys(k);
        //create_poly_message_from_round(k, 1, &k->poly_msg[0]);
    }

    long long start, end;
    start = count_cycles();

    for (i = 0; i < n; ++i) {
        if (i == 0) {
            create_player(i, playerlist[i], &public, 1);
        } else {
            create_player(i, playerlist[i], &public, 0);
        }
        create_poly_message_from_round(playerlist[i], 1, &playerlist[i]->poly_msg[0]);
    }


    /**
     * Round 1: Verify all messages received!
     */
    for (i = 0; i < n; i++) {
        for (int j = 0; j < n; ++j) {
            if (i == j) { continue; }

            if (verify_poly_message(playerlist[j], &playerlist[j]->poly_msg[0]) == 0) {
                printf("Round 1: Message from participant %d failed to verify!\n", j);
            }

        }
    }

    /**
     * Round 2:
     * X_i = (z_{i+1} - z_{i-1}) * s_i + e_i'
     */

    for (i = 0; i < n; i++) {
        int right_index = mod((uint32_t) (i - 1), n);
        calculate_x(&playerlist[mod(i + 1, n)]->z, &playerlist[right_index]->z, &playerlist[i]->secret,
                    &playerlist[i]->ep, &playerlist[i]->X);
        create_poly_message_from_round(playerlist[i], 2, &playerlist[i]->poly_msg[1]);
    }

    /**
     * Round 2:
     * Verify all received messages!
     */
    for (i = 0; i < n; i++) {
        for (int j = 0; j < n; ++j) {
            if (i == j) { continue; }

            if (verify_poly_message(playerlist[j], &playerlist[j]->poly_msg[1]) == 0) {
                printf("Round 2: Message from participant %d failed to verify!\n", j);
            }
        }
    }

    /**
     * Round 3:
     * P_{N-1} calculates b_{N-1} as
     * z_{N-2} * N * s_{N-1} + (N-1)X_i + (N-2)X_{i+1} + (N-3)X_{i+2}... X_{N-3} + e_{N-1}''
     * and then calculates a recovery vector to help with key recovery, as well as the key itself.
     */
    calculate_b_from_playerlist(playerlist, n, playerlist[n - 1]);
    poly_add(&playerlist[n - 1]->epp, &playerlist[n - 1]->b, &playerlist[n - 1]->b);
    rec_msg_ct(&playerlist[n - 1]->b, &rec);
    rec_key_ct(&rec, &playerlist[n - 1]->b, &playerlist[n - 1]->key);
    create_rec_message(playerlist[n - 1], &rec, &playerlist[n - 1]->rec_msg);

    /**
     * All players must verify the rec msg
     */

    for (i = 0; i < n - 1; i++) {
        if (verify_rec_message(playerlist[n - 1], &playerlist[n - 1]->rec_msg) == 0) {
            printf("The rec message failed verification!\n");
        }
    }

    /**
     * b_i = z_{i-1} * N * s_i + (N - 1) * X_i + (N - 2) * X_{i+1} + (N - 3) * X_{i+2} +.... + X_{i+N-2}
     */
    for (i = 0; i < n - 1; ++i) {
        calculate_b_from_playerlist(playerlist, n, playerlist[i]);
        rec_key_ct(&rec, &playerlist[i]->b, &playerlist[i]->key);
    }

    end = count_cycles();
    clock_cycles[clock_index] = end - start;

    int res = 1;

    if (equal_shared_secret(n, playerlist) == 1) {
        res = 0;
    }

    for (i = 0; i < n; ++i) {
        free(playerlist[i]);
    }

    free(playerlist);

    /**
     * No clue why I did it like this, but res non-zero => error, zero => gucci.
     */
    return res;
}

double calc_std(const uint64_t *vals, long long sum, int num) {

    double avg = (double) sum / (float) num;
    double std = 0;

    for (int i = 0; i < num; ++i) {
        std += ((double) vals[i] - avg) * ((double) vals[i] - avg);
    }
    std *= ((double) 1 / (double) (num - 1));
    std = sqrt(std);

    return std;
}

int main() {


    int player_number = 3;
    int player_max = 3;
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
            if (test_n_players(c, clock_cycles, i) > 0) {
                failures++;
            }
            long long end_c = count_cycles();
            cycles += (end_c - start_c);
            end = clock();
            sum += (double) (end - start) / CLOCKS_PER_SEC;
        }

        /*char filename[10];
        sprintf(filename, "%d.txt", c);
        FILE *fp = fopen(filename, "w");
        for (int j = 0; j < iterations; ++j) {
            fprintf(fp, "%lu,", clock_cycles[j]);
        }
        printf("Wrote to %s\n", filename);*/

        double cycle_std = calc_std(clock_cycles, cycles, iterations);
        double cycle_mean = (double) cycles / (double) (iterations * c);

        double avg_time = sum / (double) iterations;
        double failure_rate = (double) failures / (double) iterations;

        printf("PARTIES: %d.\nThere was %f percentage failures\nTotalt time: %fs. Avg: %fs\nTotal cycles: %lld. Avg: %f (%f)\n",
               c, failure_rate, sum, avg_time, cycles, cycle_mean, cycle_std);


    }

    return 0;
}
