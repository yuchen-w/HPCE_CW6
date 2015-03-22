CPPFLAGS = -I include -I opencl_sdk/include -L opencl_sdk/lib/cygwin/x86_64 
CPPFLAGS += -I /usr/include -I /usr/lib
CPPFLAGS += -Wall -std=c++11 

TBB_DIR = usr/
tbbCPPFLAGS += -I $(TBB_INC_DIR) -I $(shell pwd)/src
tbbLDFLAGS += -L $(TBB_LIB_DIR)
# Use -lm to bring in unix math library, -ltbb to bring in tbb
tbbLDLIBS = -lm -ltbb

XLINKER += -Xlinker -rpath -Xlinker $(TBB_LIB_DIR)
LDFLAGS = 
LDLIBS = -lOpenCL

# Turn on optimisations
CPPFLAGS += -O2

bin/test_opencl : src/test_opencl.cpp
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

lib/libpuzzler.a : provider/*.cpp provider/*.hpp
	cd provider && $(MAKE) all

bin/% : src/%.cpp lib/libpuzzler.a
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDLIBS) $(XLINKER) $(tbbLDFLAGS) $(tbbLDLIBS) -Llib -lpuzzler

all : bin/test_opencl bin/execute_puzzle bin/create_puzzle_input bin/run_puzzle bin/compare_puzzle_output
