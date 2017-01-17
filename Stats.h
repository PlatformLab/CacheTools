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

