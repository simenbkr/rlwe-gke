cmake_minimum_required(VERSION 3.15)
project(gake_choi C)

set(CMAKE_C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -Wextra -O3 -fomit-frame-pointer -msse2avx -march=corei7-avx -s")

add_executable(gake_choi main.c common.c common.h constant_time_tricks.c constant_time_tricks.h cpucycles.c cpucycles.h
        fft.c fft.h keccak.c keccak.h kex.c kex.h params.h poly.c poly.h randombytes.c randombytes.h reduction.c reduction.h)
target_link_libraries(gake_choi m dgs oqs)