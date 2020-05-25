cmake_minimum_required(VERSION 3.15)
project(measure_things C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -Wextra -O3 -fomit-frame-pointer -msse2avx -march=corei7-avx -s")

add_executable(measure_things main.c params.h fft.c fft.h reduction.c reduction.h poly.c poly.h randombytes.c randombytes.h kex.c kex.h constant_time_tricks.h constant_time_tricks.c)
target_link_libraries(measure_things -static dgs mpfr gmp m)