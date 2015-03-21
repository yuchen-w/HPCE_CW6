CPPFLAGS = -I include -I opencl_sdk/include -I opencl_sdk/lib/cygwin/x86_64 -Wall -std=c++11 -lOpenCL
LDFLAGS = 
# Use -lm to bring in unix math library, -ltbb to bring in tbb
LDLIBS = -lm

# Turn on optimisations
CPPFLAGS += -O2

bin/test_opencl : src/test_opencl.cpp
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)


all : bin/test_opencl 
