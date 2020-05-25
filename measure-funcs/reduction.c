#include "reduction.h"
#include "params.h"
#include <stdint.h>

static const u128 SHIFT = 72;
static const u128 F = 103765392576;

inline uint64_t barrett_128(u128 a) {
    u128 t = (a - ((a * F) >> SHIFT) * GKE_Q);

    return t < GKE_Q ? t : t - GKE_Q;
}
/**
 *
 * @param a in {0, ..., 2^64 - 1}
 * @return x in {0, ..., GKE_Q - 1} s.t. a congruent to x mod GKE_Q
 */
inline uint64_t barrett_64(uint64_t a) {
    uint64_t t = (a - ((a * F) >> SHIFT) * GKE_Q);

    return t < GKE_Q ? t : t - GKE_Q;
}

static const u128 reciprocal = 44549368224;
static const u128 factor = 1076302184447;
static const u128 mask = 1099511627775;
static const u128 shifts = 40;

uint64_t montgomery(u128 a) {

    // Does not work. Will just use Barrett because I can ¯\_(ツ)_/¯

    //convert into
    u128 b = (a << 40) % GKE_Q;

    u128 tmp = ((b & mask) * factor) & mask;

    u128 c = (b + tmp * GKE_Q) >> shifts;

    if(c >= GKE_Q) {
        return ((c - GKE_Q) * reciprocal) % GKE_Q;
    }

    return (c * reciprocal) % GKE_Q;
}