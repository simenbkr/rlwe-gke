#include <stdio.h>
#include <time.h>

#include "params.h"
#include "poly.h"
#include "common.h"
#include "kex.h"
#include "keccak.h"
#include "cpucycles.h"
#include "reduction.h"

int test_n_players(int n, uint64_t *clock_cycles, int cycle_index) {

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

    /*char *names[] = {"Alice", "Bob", "Charlie", "Dave", "Erik", "Fabian", "Garry", "Henry", "Indy", "Johnny", "Karen",
                     "Hirohito", "Larry", "Marry", "Nina", "Oscar", "Perry", "Queue", "Ronny", "Simen", "Terrance"};*/
    for (i = 0; i < n; ++i) {
        int weird = 0;
        if (i == 0) {
            weird = 1;
        }

        player *k;
        k = malloc(sizeof(player));
        //create_player(i, k, &public, weird, n);
        playerlist[i] = k;
    }

    long long start, end;
    start = count_cycles();
    for (i = 0; i < n; ++i) {
        if(i == 0){
            create_player(i, playerlist[i], &public, 1, n);
        } else {
            create_player(i, playerlist[i], &public, 0, n);
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
    }


    /**
     * Round 3:
     * P_{N-1} calculates Y_{N-1, N-1} = X_{N-1} + z_{N-2}*s_{N-1} + epp_{N-1}
     * For j=1 to N-1 computes Y_{N-1, N-1+j} = X_{N-1+j} + Y_{N-1,N-1+j-1}
     * b_{N-1} = sum from j=0 to j=N-1 of Y_{N-1, N-1+j}.
     * Creates a rec from b_{N-1}
     */

    poly_mult_mod(&playerlist[n - 2]->z, &playerlist[n - 1]->secret, &playerlist[n - 1]->Y[n - 1]);
    poly_add(&playerlist[n - 1]->epp, &playerlist[n - 1]->Y[n - 1], &playerlist[n - 1]->Y[n - 1]);
    poly_add(&playerlist[n - 1]->X, &playerlist[n - 1]->Y[n - 1], &playerlist[n - 1]->Y[n - 1]);

    for (i = 1; i < n; ++i) {
        int index_1 = mod(n - 1 + i, n);
        int index_2 = mod(n - 1 + i - 1, n);

        zero_out(&playerlist[n - 1]->Y[index_1]);
        poly_add(&playerlist[index_1]->X, &playerlist[n - 1]->Y[index_2], &playerlist[n - 1]->Y[index_1]);
    }

    for (i = 0; i < n; i++) {
        int index = mod(n - 1 + i, n);
        poly_add(&playerlist[n - 1]->b, &playerlist[n - 1]->Y[index], &playerlist[n - 1]->b);
    }

    rec_msg_ct(&playerlist[n - 1]->b, &rec);
    rec_key_ct(&rec, &playerlist[n - 1]->b, &playerlist[n - 1]->key);

    /**
     * Round 4:
     * P_i calculates Y_{i,i} = X_i + z_{i-1}*s_i
     * For j = 1 to N-1 compute Y_{i,i+j} = X_{i+j} + Y_{i,i+j-1}
     * b_i = sum from j=0 to j=N-1 of Y_{i,i+j}
     * Gets k_i from recKey(b_i, rec).
     */

    for (i = 0; i < n - 1; ++i) {
        /**
         * Y_i,i = X_i + z_i-1 * s_i
         */

         poly_mult_mod(&playerlist[mod(i - 1, n)]->z, &playerlist[i]->secret, &playerlist[i]->Y[i]);
         poly_add(&playerlist[i]->Y[i], &playerlist[i]->X, &playerlist[i]->Y[i]);

        for (int j = 1; j < n; ++j) {
            poly_add(&playerlist[mod(i + j, n)]->X, &playerlist[i]->Y[mod(i + j - 1, n)] ,&playerlist[i]->Y[mod(i + j, n)]);
        }

        for (int k = 0; k < n; ++k) {
            poly_add(&playerlist[i]->Y[mod(i + k, n)], &playerlist[i]->b, &playerlist[i]->b);
        }

        rec_key_ct(&rec, &playerlist[i]->b, &playerlist[i]->key);
    }

    end = count_cycles();
    clock_cycles[cycle_index] = end - start;

    int ret = 1;
    if(equal_secrets(playerlist, n)) {
        ret = 0;
    }

    for (i = 0; i < n; ++i) {
        free(playerlist[i]->Y);
        free(playerlist[i]);
    }

    free(playerlist);

    return ret;

}

double calc_std(const uint64_t *vals, long long sum, int num) {

    double avg = (double)sum / (float)num;
    double std = 0;

    for (int i = 0; i < num; ++i) {
        std += ((double)vals[i] - avg) * ((double)vals[i] - avg);
    }
    std *= ((double)1/(double)(num - 1));
    std = sqrt(std);

    return std;
}


double calc_mean(const uint64_t *vals, int num) {
    double sum = 0;
    for (int i = 0; i < num; ++i) {
        sum += (double) vals[i] / num;
    }
    return sum;
}

long long sumup(const uint64_t *vals, int num) {
    uint64_t s = 0;
    for (int i = 0; i < num; ++i) {
        s += vals[i];
    }
    return s;
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
            int ret = test_n_players(c, clock_cycles, i);
            long long end_c = count_cycles();
            if(ret > 0) {
                failures++;
            }
            //clock_cycles[i] = end_c - start_c;
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

        printf("Participants: %d, Failure rate: %f, Mean: %f, std: %f\n\n",
               c,
               (double)failures/(double)iterations,
               calc_mean(clock_cycles, iterations),
               calc_std(clock_cycles, sumup(clock_cycles, iterations), iterations));

    }

    return 0;
}
