SHELL=/bin/bash

CPPFLAGS += -std=c++11 -W -Wall  -g
CPPFLAGS += -O3
CPPFLAGS += -I include

tbbLDLIBS = -lm -ltbb

TBB_DIR = usr/
TBB_INC_DIR = /usr/include
TBB_LIB_DIR = /usr/lib

tbbCPPFLAGS += -I $(TBB_INC_DIR) -I $(shell pwd)/src
tbbLDFLAGS += -L $(TBB_LIB_DIR)
XLINKER += -Xlinker -rpath -Xlinker $(TBB_LIB_DIR)


lib/libpuzzler.a : provider/*.cpp provider/*.hpp
	cd provider && $(MAKE) all

bin/% : src/%.cpp lib/libpuzzler.a
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $(tbbCPPFLAGS) -o $@ $^ $(XLINKER) $(tbbLDFLAGS) $(tbbLDLIBS) $(LDFLAGS) $(LDLIBS) -Llib -lpuzzler

all : bin/execute_puzzle bin/create_puzzle_input bin/run_puzzle bin/compare_puzzle_output
