CCCC: CCCC.cc
	g++ -std=c++11 -O3 -o $@ $<  -LPerfUtils -lPerfUtils  -pthread
