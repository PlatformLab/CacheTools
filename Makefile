CXXFLAGS=-std=c++11 -O3 -g -Wall -Werror

# Output directories
SRC_DIR = src
BIN_DIR = bin

# Dependencies
PERFUTILS=PerfUtils

LIBS= -I$(PERFUTILS)/include $(PERFUTILS)/lib/libPerfUtils.a  -pthread

all: CrossCoreCommunicationCost CacheMissCostEstimator

CrossCoreCommunicationCost: CrossCoreCommunicationCost.cc Stats.h
	g++ $(CXXFLAGS) -o $@ $< $(LIBS)

CacheMissCostEstimator: CacheMissCostEstimator.cc Stats.h
	g++ $(CXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f CrossCoreCommunicationCost CacheMissCostEstimator
