SOURCE_DIR=./src
BUILD_DIR=./build
CCFLAGS=-g -pedantic -ansi -Wall -std=c++17
CC=g++

vpath %.o src

FILES=$(wildcard ./src/*)
FILE_NAMES=$(FILES:./src/%=%)
SRCS=$(wildcard ./src/*.cpp)
OBJS=$(SRCS:./src/%.cpp=./build/%.o)


TEST_OBJS=$(SRCS:./src/%.cpp=./build/%.o.t)

Chief: $(OBJS)
	$(CC) $(CCFLAGS) -o chief $(OBJS)

Test: $(TEST_OBJS) build/test.o
	$(CC) $(CCFLAGS) -o test $^

build/test.o: test/test.cpp
	$(CC) $(CCFLAGS) -I./test/src -o ./build/test.o -c ./test/test.cpp

build/%.o: src/%.cpp
	$(CC) $(CCFLAGS) -o $@ -c $<

build/%.o.t: test/src/%.cpp
	$(CC) $(CCFLAGS) -o $@ -c $<

TestBuilder: ./test/testbuilder.cpp
	$(CC) $(CCFLAGS) -o testbuilder $<
	./testbuilder $(FILE_NAMES)