cmake_minimum_required(VERSION 3.15)
project(DHGAKE C)

set(CMAKE_C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-msse2avx")
set(CMAKE_CXX_FLAGS_RELEASE "-march=corei7-avx")
set(CMAKE_CXX_FLAGS "-Wall")

add_executable(DHGAKE main.c randombytes.c randombytes.h monocypher.c monocypher.h cpucycles.c cpucycles.h)
target_link_libraries(DHGAKE -static)