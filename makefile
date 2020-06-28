SOURCE_DIR=./src
BUILD_DIR=./build
MAKER=mingw32-make.exe
CCFLAGS=-g -pedantic -ansi -Wall -std=c++17
CC=g++

vpath %.o src

FILES=$(wildcard ./src/*)
FILES2=$(FILES:./src/%=%)
SRCS=$(wildcard ./src/*.cpp)
OBJS=$(SRCS:./src/%.cpp=./build/%.o)


TEST_OBJS=$(SRCS:./src/%.cpp=./build/%.o.t)

Chief: $(OBJS)
	$(CC) $(CCFLAGS) -o chief $(OBJS)

Test: TestBuilder $(TEST_OBJS)
	$(CC) $(CCFLAGS) -o test $(TEST_OBJS)

build/test.o:
	$(CC) $(CCFLAGS) -o ./build/test.o -c ./test/test.cpp

build/%.o: src/%.cpp
	$(CC) $(CCFLAGS) -o $@ -c $<

build/%.o.t: test/src/%.cpp
	$(CC) $(CCFLAGS) -o $@ -c $<

TestBuilder: ./test/test.cpp
	$(CC) $(CCFLAGS) -o testbuilder $<
	./testbuilder $(FILES2)
