cmake_minimum_required(VERSION 3.14)
project(cache_sim C)

set(CMAKE_C_STANDARD 99)

add_library(cache_sim cache_associative_nway.c cache_associative_full.c cache_direct.c test1.c)