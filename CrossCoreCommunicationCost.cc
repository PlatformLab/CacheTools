/* Copyright (c) 2017 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include "PerfUtils/Cycles.h"
#include "PerfUtils/Util.h"
#include "PerfUtils/Stats.h"

#define USE_CAS 0
/**
 * This program attempts to measure unidirectional cross core communication
 * cost (CCCC) for all pairs of cores passed on the command line.
 *
 * The following algorithm is used here.
 *    0. Receiver starts in a different thread.
 *    1. Receiver pins itself to a core
 *    2. Sender pins itself to a core.
 *    3. Sender waits for the receiver to indicate readiness by clearing a
 *       shared memory location.
 *    4. Receiver clears the memory location to let the sender know that it is ready.
 *    5. Receiver starts polling on the shared memory location.
 *    6. Sender writes the Cycle counter to the shared memory location, and
 *       then waits for the receiver to signal that it has finished processing.
 *    7. As soon as receiver reads the timestamp, it will read the timestamp
 *       and compute and store a difference. It will then clear the flag to
 *       signal to the sender that it is ready for the next sending.
 */

#define NUM_WARMUP 10000

#define NUM_RUNS 3000000
#define NUM_COUNTS NUM_RUNS

using PerfUtils::Cycles;

void recv(uint64_t coreId, std::atomic<uint64_t>* timestamp,
        uint64_t *cycleDifferences){
    PerfUtils::Util::pinThreadToCore(coreId);
    uint64_t endTime;

    for (int i = 0; i < NUM_WARMUP; i++) {
        *timestamp = 0;
        while (*timestamp == 0);
    }
    for (int i = 0; i < NUM_RUNS; i++) {
        *timestamp = 0;
        while (*timestamp == 0);
        endTime = Cycles::rdtsc();
        uint64_t timeDiff = endTime - *timestamp;
        if (timeDiff < NUM_COUNTS - 1)
            cycleDifferences[timeDiff]++;
        else
            cycleDifferences[NUM_COUNTS - 1]++;
    }
}

void send(uint64_t coreId, std::atomic<uint64_t>* timestamp) {
    PerfUtils::Util::pinThreadToCore(coreId);
    for (int i = 0; i < NUM_RUNS + NUM_WARMUP; i++) {
        while (*timestamp != 0);
#if USE_CAS
        uint64_t oldValue = 0L;
		timestamp->compare_exchange_strong(oldValue, Cycles::rdtsc());
#else
        *timestamp = Cycles::rdtsc();
#endif
    }
}

void ensureDirectory(const char* dir) {
    struct stat st = {0};

    if (stat(dir, &st) == -1) {
        mkdir(dir, 0700);
    }
}

void translateData(uint64_t* countArray, uint64_t* rawdata) {
    int k = 0;
    for (uint64_t i = 0; i < NUM_COUNTS; i++)
        for (uint64_t j = 0; j < countArray[i]; j++)
            rawdata[k++] = i;
    assert(k == NUM_RUNS);
}

int main(int argc, const char** argv){
    // Read arguments
    if (argc < 2) {
        printf("Usage: ./CCCC <cpu_range> [datadir]\n");
        return 1;
    }
    const char* datadir = NULL;
    if (argc > 2) {
        datadir = argv[2];
        ensureDirectory(datadir);
    }

    std::vector<int> cores = PerfUtils::Util::parseRanges(argv[1]);
    void* timestampHolder =
        PerfUtils::Util::cacheAlignAlloc(sizeof(std::atomic<uint64_t>));;
    if (timestampHolder == NULL) {
        printf("Failed to obtain cache-aligned memory for timestamp\n");
    }
    new (timestampHolder) std::atomic<uint64_t>();
    std::atomic<uint64_t>* timestamp =
        reinterpret_cast< std::atomic<uint64_t>* >(timestampHolder);

    uint64_t* rawdata = (uint64_t*)malloc(NUM_RUNS*sizeof(uint64_t));
    uint64_t* countArray = (uint64_t*)malloc(NUM_COUNTS*sizeof(uint64_t));
    for (size_t i = 0; i < cores.size(); i++) {
        for (size_t k = 0; k < cores.size(); k++) {
            if (cores[i] == cores[k]) continue;
            memset(countArray, 0, NUM_COUNTS * sizeof(uint64_t));
            *timestamp = 1;
            std::thread r(recv, cores[k], timestamp, countArray);
            send(cores[i], timestamp);
            r.join();
            // Analyze data
            PerfUtils::Util::serialize();
            translateData(countArray, rawdata);
            char label[1024];
            sprintf(label, "Core %d to Core %d", cores[i], cores[k]);
            // Translate cycles to nanoseconds
            for (int i = 0; i < NUM_RUNS; i++)
                rawdata[i] = Cycles::toNanoseconds(rawdata[i]);
            printStatistics(label, rawdata, NUM_RUNS, datadir);
            PerfUtils::Util::serialize();
        }
    }
    free(rawdata);
    free(countArray);
    free(timestampHolder);
}
