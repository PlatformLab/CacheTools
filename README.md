# CPU Cache Measurement Tools

This project consists of two tools for performing measurements on CPU Caches.

The first one is CacheMissCostEstimator, which takes an array size as an
argument and performs random accesses on an array of this size, and measures
latencies. This tool is not particularly accurate so its results should only be
used as an approximation of cache miss cost.

The second one is CrossCoreCommunicationCost, which performs pairwise
measurements of the time it takes to write a 64-bit integer to a memory
location from one core and read it from another core.

Both tools take an optional datadir directory. If this option is given, the
tool puts the raw measurements (in human-readable format) into one or more
files in the datadir.
