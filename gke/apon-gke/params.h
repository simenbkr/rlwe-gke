#ifndef PARAMS_H
#define PARAMS_H

/**
 * The paper by Apon et al requires that
 * 1: APON_N __must__ be a power of 2.
 * 2: APON_Q __must__ be chosen such that APON_Q mod 2 * APON_N === 1
 * 3: The Normal distributions X_s, X_e return small values for the coefficients, for some definition of small.
 */

/**
 * Parameter set below should give 128 bit post-quantum security, with an error rate of 2^(-63) for N=5.
 */

#define APON_N 1024
//#define APON_Q 4294924289
//#define APON_Q 4294967295

//#define APON_Q 4474796033
//#define APON_Q 34151682049 // <-- 35-bit integer prime s.t. === 1 mod 2*1024.

//#define APON_Q 34052579329 // <-- 34-bit integer prime ^
//#define APON_Q 18189187073 // <-- "small" 34-bit prime. Error rate 0.008 for N = 5, SIGMA_1 = 2, SIGMA_2 = 18536480. 1000 trials.

//#define APON_Q 19851896833

#define APON_Q 45510033409
#define N_INVERSE 45465590017 //1024^-1 MOD 45510033409


/**
Found 45510033409 = 173607*2^18 + 1 to be prime
  Primitive root of unity: 23
  Root of unity for transformations up to 2^18: 31636173194
  Root of unity for transformations up to 2^18: 29410191894
  Root of unity for transformations up to 2^17: 22661636131
  Root of unity for transformations up to 2^16: 15688382514
  Root of unity for transformations up to 2^15: 157680031
  Root of unity for transformations up to 2^14: 41744222899
  Root of unity for transformations up to 2^13: 32776691131
  Root of unity for transformations up to 2^12: 35141411059
  Root of unity for transformations up to 2^11: 42305359955
  Root of unity for transformations up to 2^10: 3316187664
  Root of unity for transformations up to 2^9: 36043823599
  Root of unity for transformations up to 2^8: 40262948345
  Root of unity for transformations up to 2^7: 36271791672
  Root of unity for transformations up to 2^6: 17444836549
  Root of unity for transformations up to 2^5: 31034634912
  Root of unity for transformations up to 2^4: 34320582150
  Root of unity for transformations up to 2^3: 7350412184
  Root of unity for transformations up to 2^2: 45510033408
  Scaling factor: 22755016705
 */

//#define APON_Q 19341699073

//#define APON_Q 17416704001


#define SIGMA_1 2
//#define SIGMA_2 463410000
//#define SIGMA_2 115855
//#define SIGMA_2 61537
//#define SIGMA_2 22243753
//#define SIGMA_2 1853646
//#define SIGMA_2 3
//#define SIGMA_2 18536480 * 3.6
//#define SIGMA_2 18536480 * 3.6203 * 2.35
//#define SIGMA_2 157286640.0
//#define SIGMA_2 157286640
#define SIGMA_2 94371960
#define SIGMA_2_squared 8906066834241600
#define MAX_SAMPLER 33554432

#define MAX_WIDTH 16


#endif //PARAMS_H