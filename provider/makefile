SHELL=/bin/bash

CPPFLAGS += -std=c++11 -W -Wall  -g
CPPFLAGS += -O3
CPPFLAGS += -I ../include ../opencl_sdk/include

puzzles.o : *.hpp

../lib/libpuzzler.a : puzzles.o
	-mkdir -p ../lib
	ar cr ../lib/libpuzzler.a puzzles.o

all : ../lib/libpuzzler.a
