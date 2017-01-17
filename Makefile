all: CCCC CMC

CCCC: CCCC.cc
	g++ -std=c++11 -O3 -o $@ $<  PerfUtils/libPerfUtils.a -pthread

CMC: CMC.cc
	g++ -std=c++11 -O3 -o $@ $<  PerfUtils/libPerfUtils.a -pthread

clean:
	rm -f CCCC CMC
