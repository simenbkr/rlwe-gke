cmake_minimum_required(VERSION 3.15)
project(GAKE_apon C)
set(CMAKE_C_STANDARD 99)
set(TARGET_NAME GAKE_apon)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -Wextra -O3 -fomit-frame-pointer -msse2avx -march=corei7-avx -s")

add_executable(GAKE_apon main.c randombytes.c poly.c kem.c kem.h common.c common.h keccak.c keccak.h
        constant_time_tricks.c constant_time_tricks.h cpucycles.c cpucycles.h fft.c fft.h reduction.c reduction.h)
target_link_libraries(GAKE_apon -static dgs oqs mpfr gmp m)