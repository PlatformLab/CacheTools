all: CrossCoreCommunicationCost CacheMissCostEstimator

CrossCoreCommunicationCost: CrossCoreCommunicationCost.cc Stats.h
	g++ -std=c++11 -g -O3 -o $@ $<  PerfUtils/libPerfUtils.a -pthread

CacheMissCostEstimator: CacheMissCostEstimator.cc Stats.h
	g++ -std=c++11 -g -O3 -o $@ $<  PerfUtils/libPerfUtils.a -pthread

clean:
	rm -f CrossCoreCommunicationCost CacheMissCostEstimator
