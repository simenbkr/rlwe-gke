cmake_minimum_required(VERSION 3.15)
project(gke_choi C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -Wextra -O3 -fomit-frame-pointer -msse2avx -march=corei7-avx -g")

add_executable(gke_choi main.c randombytes.c poly.c common.c common.h keccak.c keccak.h kex.c kex.h fft.c fft.h
        cpucycles.h cpucycles.c constant_time_tricks.h constant_time_tricks.c reduction.c reduction.h)
target_link_libraries(gke_choi -static dgs mpfr gmp m)