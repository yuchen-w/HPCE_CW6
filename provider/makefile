SHELL=/bin/bash

CPPFLAGS += -std=c++11 -W -Wall  -g
CPPFLAGS += -O3
CPPFLAGS += -I ../include

tbbLDLIBS = -lm -ltbb

TBB_DIR = usr/
TBB_INC_DIR = /usr/include
TBB_LIB_DIR = /usr/lib

tbbCPPFLAGS += -I $(TBB_INC_DIR) -I $(shell pwd)/src
tbbLDFLAGS += -L $(TBB_LIB_DIR)
XLINKER += -Xlinker -rpath -Xlinker $(TBB_LIB_DIR)

puzzles.o : *.hpp
#$(CXX) $(tbbCPPFLAGS) $^ -o $@ $(XLINKER) $(tbbLDFLAGS) $(tbbLDLIBS)

../lib/libpuzzler.a : puzzles.o
	-mkdir -p ../lib
	$(CXX) $(tbbCPPFLAGS) $^ -o $@ $(XLINKER) $(tbbLDFLAGS) $(tbbLDLIBS)
	ar cr ../lib/libpuzzler.a puzzles.o

all : ../lib/libpuzzler.a
