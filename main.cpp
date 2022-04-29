//
// Created by Mikhail Gorshkov on 29.04.2022.
//
//

#include <pthread.h>
#include <thread>
#include <iostream>
#include <memory>
#include <sched.h>
#include <x86intrin.h>
#include <algorithm>
#include <cstring>
#include <vector>

//#define DEBUG_LOG

static inline std::uint64_t rdtsc() {
    return __rdtsc();
}

static inline void setThreadAffinity(pthread_t thread, int cpu) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    int rc = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << std::endl;
    }
}

struct CacheLine {
    uint64_t timestamp;
    unsigned char data[CACHELINE_SIZE - sizeof(int64_t)];
} __attribute((aligned(CACHELINE_SIZE)));

static int64_t getReport(std::unique_ptr<CacheLine[]> cacheLineArray, std::size_t size, double freq) {
    std::vector<uint64_t> report;
    for (std::size_t i = 0; i < size - 1; i += 2) {
#ifdef DEBUG_LOG
        std::cout << i << " " << cacheLineArray[i].timestamp << " " << cacheLineArray[i + 1].timestamp << std::endl;
#endif
        if (cacheLineArray[i + 1].timestamp != cacheLineArray[i].timestamp)
            std::cerr << "Incorrect data, offset=" << i << std::endl;
        report.push_back(cacheLineArray[i + 2].timestamp - cacheLineArray[i].timestamp);
    }
    std::sort(report.begin(), report.end());
    std::size_t reportSize = size / 2 - 1;
    std::size_t medianPos = reportSize / 2;
    uint64_t median = reportSize % 2 == 0 ?
                      (report[medianPos - 1] + report[medianPos]) / 2 :
                        report[medianPos];
#ifdef DEBUG_LOG
    std::cout << "Median=" << median << std::endl;
#endif
    auto roundtrip_nanoseconds = static_cast<int64_t>(static_cast<double>(median) / freq);
    return roundtrip_nanoseconds;
}

// One write/read loop iteration:
//------------------------------------------------------------
//                stage 1 | stage 2 | stage 3 | stage 4 | stage 1
// publisher      write   | wait    | wait    | read    | ...
// consumer       wait    | read    | write   | wait    | ...
//-------------------------------------------------------------
// Data looks like this
// TS counter 1 (write)
// TS counter 1 (read)
// TS counter 2 (write)
// TS counter 2 (read)
// …
// TS counter 1000000 (write)
// TS counter 1000000 (read)
// read counters are the ones read by the consumer and duplicated
// Then we calculate the diffs
// TS diff 1 = TS counter 2 - TS counter 1
// TS diff 2 = TS counter 3 - TS counter 2
// …
// TS diff 999999 = TS counter 1000000 - TS counter 999999

static int64_t runBenchmark(int num_messages, int core1, int core2, double freq) {
    const std::size_t size = num_messages * 2;
    std::unique_ptr<CacheLine[]> cacheLineArray{new CacheLine[size]()};
    const auto startPtr = &cacheLineArray[0];
    const auto endPtr = startPtr + size;

    // consumer
    std::thread consumer{[startPtr, endPtr, core1] {
        setThreadAffinity(pthread_self(), core1);

        auto ptr = startPtr;
        // same cacheline for read/write
        while (ptr < endPtr) {
            CacheLine cacheLine;
            do {
                std::memcpy(&cacheLine, ptr, sizeof(CacheLine)); // read
            } while (cacheLine.timestamp == 0);
            ++ptr;
            std::memcpy(ptr++, &cacheLine, sizeof(CacheLine)); // write
        }
    }};

    setThreadAffinity(pthread_self(), core2);

    // publisher
    auto ptr = startPtr;
    while (ptr < endPtr) {
        CacheLine cacheLine{rdtsc()};
        {
            std::memcpy(ptr++, &cacheLine, sizeof(CacheLine)); // write
        }

        CacheLine cacheLineRead;
        do {
            std::memcpy(&cacheLineRead, ptr, sizeof(CacheLine)); // read
        } while (cacheLineRead.timestamp != cacheLine.timestamp);
        ++ptr;
    }

    consumer.join();

    return getReport(std::move(cacheLineArray), size, freq) / 2; // single trip
}

int main(int argc, char** argv) {
    const char* kAbout =
            "A tool for measuring latency of a single cache line-length message delivery between different cores of a CPU";
    const char* kUsage = "Usage: ./measure_cache_line_delivery_ns [<num_messages> (default 1M)] "
                         "[<consumer cpu id> (default 0)] [<producer cpu id> (default 1)] [<cpu freq (HGz)> (default 3.0)]\n"
                         "example: ./measure_cache_line_delivery_ns 1000 5 6 4.0";

    std::cerr << kAbout << std::endl;
    std::cerr << "Cache line size is " << CACHELINE_SIZE << "bytes" << std::endl;
    std::cerr << kUsage << std::endl;

    if (argc == 2 && !strcmp(argv[1], "-h")) {
        return 1;
    }

    int num_messages = argc > 1 ? std::stoi(argv[1]) : 1000000;
    if (num_messages < 2) {
        std::cerr << "num_messages should be >= 2" << std::endl;
        return 1;
    }
    int core1 = argc > 2 ? std::stoi(argv[2]) : 0;
    int core2 = argc > 3 ? std::stoi(argv[3]) : 1;
    double freq = argc > 4 ? std::stod(argv[4]) : 3.0;

    std::cout << "Running the benchmark with parameters: num_messages=" << num_messages << ", core1=" << core1 <<
        ", core2=" << core2 << ", freq=" << freq << "GHz..." << std::endl;

    auto nanoseconds = runBenchmark(num_messages, core1, core2, freq);

    std::cout << "Single trip is " << nanoseconds << "ns " << std::endl;

    return 0;
}