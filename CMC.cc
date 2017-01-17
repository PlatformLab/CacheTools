#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "PerfUtils/Cycles.h"

#define NUM_RUNS 100000

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
    size_t size = atoi(argv[1]);
    uint8_t *memory = (uint8_t *) malloc(size);
    srand(time(NULL));

    size_t sum = 0;
    uint64_t startTime = Cycles::rdtsc();

    for (int i = 0; i < NUM_RUNS; i++) {
        sum += memory[rand() % size];
    }

    uint64_t delta = Cycles::rdtsc() - startTime;
    printf("Average cost of randomly accessing an array of size %lu bytes:"
            "%lu\n", size, Cycles::toNanoseconds(delta / NUM_RUNS));

    free(memory);
    FILE* fp = fopen("/dev/null", "w");
    fprintf(fp, "%lu\n", sum);
    fclose(fp);
}
