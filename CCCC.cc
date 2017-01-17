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
#include "Stats.h"

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
        cycleDifferences[i] = endTime - *timestamp;
    }
}

void send(uint64_t coreId, std::atomic<uint64_t>* timestamp) {
    PerfUtils::Util::pinThreadToCore(coreId);
    for (int i = 0; i < NUM_RUNS + NUM_WARMUP; i++) {
        while (*timestamp != 0);
        *timestamp = Cycles::rdtsc();
    }
}

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::vector<int> parseCores(const char* coreDesc) {
    std::vector<int> cores;
    std::vector<std::string> ranges = split(coreDesc, ',');
    for (int i = 0; i < ranges.size(); i++) {
        if (ranges[i].find("-") == std::string::npos)
            cores.push_back(atoi(ranges[i].c_str()));
        else {
            auto bounds = split(ranges[i], '-');
            for (int k = atoi(bounds[0].c_str()); k <= atoi(bounds[1].c_str());
                    k++)
                cores.push_back(k);
        }
    }
    return cores;
}

void ensureDirectory(const char* dir) {
    struct stat st = {0};

    if (stat(dir, &st) == -1) {
        mkdir(dir, 0700);
    }
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

    std::vector<int> cores = parseCores(argv[1]);
    std::atomic<uint64_t> timestamp;
    uint64_t* rawdata = (uint64_t*)malloc(NUM_RUNS*sizeof(uint64_t));
    for (int i = 0; i < cores.size(); i++) {
        for (int k = 0; k < cores.size(); k++) {
            if (cores[i] == cores[k]) continue;
            timestamp = 1;
            std::thread r(recv, cores[k], &timestamp, rawdata);
            send(cores[i], &timestamp);
            r.join();
            // Analyze data
            PerfUtils::Util::serialize();
            char label[1024];
            sprintf(label, "Core %d to Core %d", cores[i], cores[k]);
            printStatistics(label, rawdata, NUM_RUNS, datadir);
            PerfUtils::Util::serialize();
        }
    }
    free(rawdata);
}
