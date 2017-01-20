#include <math.h>

int compare(const void * a, const void * b)
{
    if (*(uint64_t*)a == *(uint64_t*)b) return 0;
    return  *(uint64_t*)a < *(uint64_t*)b ? -1 : 1;
}

void printStatistics(const char* label, uint64_t* rawdata, size_t count,
        const char* datadir) {
    static bool headerPrinted = false;
    qsort(rawdata, count, sizeof(uint64_t), compare);
    uint64_t sum = 0;
    for (int i = 0; i < count; i++)
        sum += rawdata[i];
    uint64_t avg = sum / count;

    if (!headerPrinted) {
        puts("Benchmark,Count,Avg,Median,Min,99%,"
                "99.9%,99.99%,Max");
        headerPrinted = true;
    }
    printf("%s,%zu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n", label, count, avg,
            rawdata[count / 2], rawdata[0], rawdata[(int)(count*0.99)],
            rawdata[(int)(count*0.999)],rawdata[(int)(count*0.9999)],
            rawdata[count-1]);

    // Dump the data out
    if (datadir != NULL) {
        char buf[1024];
        sprintf(buf, "%s/%s", datadir, label);
        FILE* fp = fopen(buf, "w");
        for (int i = 0; i < count; i++)
            fprintf(fp, "%lu\n", rawdata[i]);
        fclose(fp);
    }
}

void printHistogram(uint64_t* rawdata, size_t count, uint64_t lowerbound,
        uint64_t upperbound, uint64_t step) {
    size_t numBuckets = (upperbound - lowerbound) / step + 1;
    uint64_t *buckets = (uint64_t *) calloc(numBuckets, sizeof(uint64_t));
    for (int i = 0; i < count; i++) {
        bool foundBucket = false;
        for (uint64_t k = lowerbound; k < upperbound; k += step) {
            if (rawdata[i] < k + step) {
                buckets[(k - lowerbound) / step]++;
                foundBucket = true;
                break;
            }
        }
        if (!foundBucket) {
            buckets[numBuckets-1]++;
        }
    }

    for (uint64_t k = lowerbound; k < upperbound; k += step) {
        printf("%lu-%lu: %lu\n", k, k + step, buckets[(k - lowerbound) / step]);
    }
    printf("%lu+: %lu\n", upperbound, buckets[numBuckets -1]);
    free(buckets);
}
