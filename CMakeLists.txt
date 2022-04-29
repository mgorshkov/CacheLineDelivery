cmake_minimum_required(VERSION 3.18)
project(CacheLineDelivery)

set(CMAKE_CXX_STANDARD 14)
set(THREADS_PREFER_PTHREAD_FLAG ON)

exec_program(getconf ARGS LEVEL1_DCACHE_LINESIZE OUTPUT_VARIABLE CACHELINE_SIZE)
add_compile_definitions(CACHELINE_SIZE=${CACHELINE_SIZE})

find_package(Threads REQUIRED)

add_executable(measure_cache_line_delivery_ns main.cpp)
target_link_libraries(measure_cache_line_delivery_ns PRIVATE Threads::Threads)
