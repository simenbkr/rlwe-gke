#ifndef PARAMS_H
#define PARAMS_H

/**
 * Parameter set below should give 128 bit post-quantum security, with an error rate of 2^(-63) for N=5.
 */

#define GKE_N 1024
#define GKE_Q 45510033409
#define N_INVERSE 45465590017 //1024^-1 MOD 45510033409
#define LENGTH_PACKED 8192
#define SIGMA_1 2
#define SIGMA_2 94371960
#define SIGMA_2_squared 8906066834241600
#define MAX_SAMPLER 33554432
#define MAX_WIDTH 16

#endif //PARAMS_H
