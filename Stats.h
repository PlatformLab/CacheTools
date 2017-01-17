#include <math.h>

int compare(const void * a, const void * b)
{
    if (*(uint64_t*)a == *(uint64_t*)b) return 0;
    return  *(uint64_t*)a < *(uint64_t*)b ? -1 : 1;
}

void printStatistics(const char* label, uint64_t* rawdata, size_t count,
        const char* datadir) {
    qsort(rawdata, count, sizeof(uint64_t), compare);
    uint64_t sum = 0;
    for (int i = 0; i < count; i++)
        sum += rawdata[i];
    uint64_t avg = sum / count;

    double stddev = 0;
    for (size_t i=0; i<count; i++) {
        if (rawdata[i] > avg) {
            stddev += (rawdata[i] - avg) * (rawdata[i] - avg);
        } else {
            stddev += (avg- rawdata[i]) * (avg - rawdata[i]);
        }
    }
    stddev /= count;
    stddev = sqrt(stddev);

    // count,avg,stddev,median,min,max
    printf("%s,%zu,%lu,%f,%lu,%lu,%lu\n", label, count, avg, stddev,
            rawdata[count / 2], rawdata[0],rawdata[count-1]);

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

void printHistogram(uint64_t* rawdata, size_t count, uint64_t lowerbound, uint64_t upperbound, uint64_t step) {
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
