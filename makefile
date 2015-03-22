#TBB
CPPFLAGS = -I include -Wall -std=c++11 
LDLIBS = -lm -ltbb

# Turn on optimisations
CPPFLAGS += -O2

#OpenCL
CPPFLAGS += -I include -I opencl_sdk/include 
LDLIBS += -lOpenCL

#TBB
TBB_DIR = /usr
TBB_INC_DIR = $(TBB_DIR)/include
TBB_LIB_DIR = $(TBB_DIR)/lib

#TBB_DIR = C:/HPCE/tbb
#TBB_INC_DIR = $(TBB_DIR)/include
##TBB_LIB_DIR = $(TBB_DIR)/lib/intel64/gcc4.4

CPPFLAGS += -I $(TBB_INC_DIR) -I $(shell pwd)/src
LDFLAGS += -L $(TBB_LIB_DIR)

XLINKER += -Xlinker -rpath -Xlinker $(TBB_LIB_DIR)



bin/test_opencl : src/test_opencl.cpp
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

lib/libpuzzler.a : provider/*.cpp provider/*.hpp
	cd provider && $(MAKE) all

bin/% : src/%.cpp lib/libpuzzler.a
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDLIBS) $(XLINKER) $(LDFLAGS) -Llib -lpuzzler

all : bin/test_opencl bin/execute_puzzle bin/create_puzzle_input bin/run_puzzle bin/compare_puzzle_output
