# Measuring one cache line delivery latency

This sample measures single-trip delivery latency of one cache line-sized block between two CPUs of the same core.

# Requirements
* x86

* linux

* multicore CPU

* a C++14-compatible compiler

* cmake 3.18 or later

# Build

1. Create build folder
```
mkdir build && cd build
```

2. Create cmake auxiliary files
```
cmake ..
```

3. Build
```
cmake --build .
```

# Run
## Prerequisite
CPU frequency of the cores involved needs to be precalculated before starting.

It can be done by launching

```
cat /proc/cpuinfo | grep Hz
model name	: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
cpu MHz		: 800.000
model name	: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
cpu MHz		: 2800.000
model name	: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
cpu MHz		: 800.000
model name	: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
cpu MHz		: 800.000
model name	: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
cpu MHz		: 2794.449
model name	: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
cpu MHz		: 800.000
model name	: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
cpu MHz		: 800.000
model name	: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
cpu MHz		: 800.000

```

## Core selection
Select two cores on which you'll be launching the benchmark (ex, cores 2, 3).

## Launch the benchmark
```
./measure_cache_line_delivery_ns [<num_messages> (default 1M)] [<consumer cpu id> (default 0)] [<producer cpu id> (default 1)] [<cpu freq (HGz)> (default 3.0)]
```
Examples:
```
./measure_cache_line_delivery_ns 1000 2 3 4.0
A tool for measuring latency of a single cache line-length message delivery between different cores of a CPU
Usage: ./measure_cache_line_delivery_ns [<num_messages> (default 1M)] [<consumer cpu id> (default 0)] [<producer cpu id> (default 1)] [<cpu freq (HGz)> (default 3.0)]
example: ./measure_cache_line_delivery_ns 1000 5 6 4.0
Running the benchmark with parameters: num_messages=1000, core1=2, core2=3, freq=4GHz...
Single trip is 411ns 
```
```
./measure_cache_line_delivery_ns
A tool for measuring latency of a single cache line-length message delivery between different cores of a CPU
Usage: ./measure_cache_line_delivery_ns [<num_messages> (default 1M)] [<consumer cpu id> (default 0)] [<producer cpu id> (default 1)] [<cpu freq (HGz)> (default 3.0)]
example: ./measure_cache_line_delivery_ns 1000 5 6 4.0
Running the benchmark with parameters: num_messages=1000000, core1=0, core2=1, freq=3GHz...
Single trip is 399ns 

```