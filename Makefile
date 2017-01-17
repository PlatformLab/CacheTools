all: CCCC CMC

CCCC: CCCC.cc Stats.h
	g++ -std=c++11 -g -O3 -o $@ $<  PerfUtils/libPerfUtils.a -pthread

CMC: CMC.cc Stats.h
	g++ -std=c++11 -g -O3 -o $@ $<  PerfUtils/libPerfUtils.a -pthread

clean:
	rm -f CCCC CMC
