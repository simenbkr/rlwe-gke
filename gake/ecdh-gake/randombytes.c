#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "randombytes.h"
#include <stdint.h>

static int fd = -1;

void _randombytes(unsigned char *x,unsigned long long xlen)
{
    int i;

    if (fd == -1) {
        for (;;) {
            fd = open("/dev/urandom",O_RDONLY);
            if (fd != -1) break;
            sleep(1);
        }
    }

    while (xlen > 0) {
        if (xlen < 1048576) i = xlen; else i = 1048576;

        i = read(fd,x,i);
        if (i < 1) {
            sleep(1);
            continue;
        }

        x += i;
        xlen -= i;
    }
}

uint64_t _random_u64() {

    uint64_t result = 0;
    int i = 0;
    unsigned char buf[sizeof(uint64_t)];
    _randombytes(buf, sizeof(uint64_t));

    result  = ((uint64_t)(buf[i++] & 0xff)) << 070;
    result |= ((uint64_t)(buf[i++] & 0xff)) << 060;
    result |= ((uint64_t)(buf[i++] & 0xff)) << 050;
    result |= ((uint64_t)(buf[i++] & 0xff)) << 040;
    result |= ((uint64_t)(buf[i++] & 0xff)) << 030;
    result |= ((uint64_t)(buf[i++] & 0xff)) << 020;
    result |= ((uint64_t)(buf[i++] & 0xff)) << 010;
    result |= ((uint64_t)(buf[i++] & 0xff));

    return result;
}