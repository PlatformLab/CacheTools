#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "PerfUtils/Cycles.h"
#include "Stats.h"

#define NUM_RUNS 3000000

// Uncomment the following line to meausre only overheads
//#define NULL_MEASURE

using PerfUtils::Cycles;

void ensureDirectory(const char* dir) {
    struct stat st = {0};

    if (stat(dir, &st) == -1) {
        mkdir(dir, 0700);
    }
}

/**
 * Cache Miss Cost: This tool attempts to measure the cost of various levels of
 * cache misses by gradually increasing the size of an in-memory iterated over
 * in random order to avoid hardware prefetching effects.
 */
int main(int argc, char** argv){
    if (argc < 2) {
        printf("Usage: ./CMC <array_size_in_bytes> [datadir]\n");
        return 1;
    }

    const char* datadir = NULL;
    if (argc > 2) {
        datadir = argv[2];
        ensureDirectory(datadir);
    }

    size_t count =  atoi(argv[1])/ sizeof(uint8_t*);
    if (count < 1) {
        printf("Array size must be at least %lu bytes.\n", sizeof(uint8_t*));
        return 1;
    }
    uint8_t **memory = (uint8_t **) malloc(count * sizeof(uint8_t*));

    // Single cycle random permutation algorithm
    srand(time(NULL));
    for (size_t i = 0; i < count; i++) {
        memory[i] = (uint8_t*) &memory[i];
    }

    for (size_t i = 0; i < count - 1; i++) {
        size_t swapWithIndex = (size_t) ((rand() % (count - i - 1)) + (i + 1));
        uint8_t* val = memory[swapWithIndex];
        memory[swapWithIndex] = memory[i];
        memory[i] = val;
    }


    // Begin measurements
    uint64_t *deltas = new uint64_t[NUM_RUNS];
    uint64_t startTime;
    uint64_t delta;

    uint8_t **cur = memory;
    for (int i = 0; i < NUM_RUNS; i++) {
        startTime = Cycles::rdtsc();
        #ifndef NULL_MEASURE
        cur = (uint8_t**) *cur;
        #endif
        delta = Cycles::rdtsc() - startTime;
        deltas[i] = delta;
    }
    free(memory);
    printStatistics("Cache Statistics", deltas, NUM_RUNS, datadir);

    // Force usage of output value so that compiler doesn't optimize away our
    // loop.
    FILE* fp = fopen("/dev/null", "w");
    fprintf(fp, "%p\n", cur);
    fclose(fp);
    delete [] deltas;
}
