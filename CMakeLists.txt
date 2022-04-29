cmake_minimum_required(VERSION 3.18)
project(CacheLineDelivery)

set(CMAKE_CXX_STANDARD 14)

add_executable(measure_cache_line_delivery_ns main.cpp)
