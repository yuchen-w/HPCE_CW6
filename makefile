SHELL=/bin/bash

OPENCL_INCLUDES = -I src -I opencl_sdk/include -I include -L opencl_sdk/lib/cygwin/x86_64 -lOpenCL
OPENCL_LIBS = -lOpenCL -L opencl_sdk/lib/cygwin/x86_64

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




bin/test_opencl : src/test_opencl.cpp
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $(OPENCL_INCLUDES) $^ -o $@ $(LDFLAGS) $(OPENCL_LIBS)

lib/libpuzzler.a : provider/*.cpp provider/*.hpp
	cd provider && $(MAKE) all

bin/% : src/%.cpp lib/libpuzzler.a
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $(tbbCPPFLAGS) $(OPENCL_INCLUDES) -o $@ $^ $(XLINKER) $(tbbLDFLAGS) $(tbbLDLIBS) $(LDFLAGS) $(LDLIBS) $(OPENCL_LIBS) -Llib -lpuzzler

all : bin/execute_puzzle bin/create_puzzle_input bin/run_puzzle bin/compare_puzzle_output
