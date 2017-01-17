#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "PerfUtils/Cycles.h"

#define NUM_RUNS 10000

using PerfUtils::Cycles;
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


    uint8_t **cur = memory;
    uint64_t startTime = Cycles::rdtsc();
    for (int i = 0; i < NUM_RUNS; i++) {
        cur = (uint8_t**) *cur;
    }
    uint64_t delta = Cycles::rdtsc() - startTime;

    printf("Average cost of randomly accessing an array of size %lu bytes: "
            "%lu ns\n", count*sizeof(uint8_t*),
            Cycles::toNanoseconds(delta / NUM_RUNS));
    free(memory);

    // Force usage of output value so that compiler doesn't optimize away our
    // loop.
    FILE* fp = fopen("/dev/null", "w");
    fprintf(fp, "%p\n", cur);
    fclose(fp);
}
