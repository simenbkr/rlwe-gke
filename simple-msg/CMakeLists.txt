#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#SET(BUILD_SHARED_LIBRARIES OFF)
#SET(CMAKE_EXE_LINKER_FLAGS "-static")


cmake_minimum_required(VERSION 3.15)
project(simple_msg C)

set(TARGET_NAME simple_msg)
set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -Wextra -O3 -fomit-frame-pointer -msse2avx -march=corei7-avx")

add_executable(simple_msg main.c randombytes.c randombytes.h poly.c kem.c kem.h common.c common.h keccak.c keccak.h
        constant_time_tricks.c constant_time_tricks.h cpucycles.c cpucycles.h fft.c fft.h monocypher.c monocypher.h)
target_link_libraries(simple_msg -static dgs oqs mpfr gmp m pthread)
