CPPFLAGS = -I include -I opencl_sdk/include -I opencl_sdk/lib/cygwin/x86_64 -Wall -std=c++11 -lOpenCL
CPPFLAGS += -I /usr/include -I /usr/lib

TBB_DIR = usr/
tbbCPPFLAGS += -I $(TBB_INC_DIR) -I $(shell pwd)/src
tbbLDFLAGS += -L $(TBB_LIB_DIR)
tbbLDLIBS = -lm -ltbb

XLINKER += -Xlinker -rpath -Xlinker $(TBB_LIB_DIR)
LDFLAGS = 
# Use -lm to bring in unix math library, -ltbb to bring in tbb
LDLIBS = -lm

# Turn on optimisations
CPPFLAGS += -O2

bin/test_opencl : src/test_opencl.cpp
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

lib/libpuzzler.a : provider/*.cpp provider/*.hpp
	cd provider && $(MAKE) all

bin/% : src/%.cpp lib/libpuzzler.a
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) -o $@ $^ $(XLINKER) $(tbbLDFLAGS) $(tbbLDLIBS) $(LDFLAGS) $(LDLIBS) -Llib -lpuzzler

all : bin/test_opencl 
