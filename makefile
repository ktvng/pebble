SOURCE_DIR=./src
BUILD_DIR=./build
MAKER=mingw32-make.exe
CCFLAGS=-g -pedantic -ansi -Wall -std=c++17
CC=g++

vpath %.o src

SRCS=$(wildcard ./src/*.cpp)
OBJS=$(SRCS:./src/%.cpp=./build/%.o)

TEST_SRCS=$(wildcard ./test/*.cpp)
TEST_OBJS=$(TEST_SRCS:./test/%.cpp=./build/%.o)

Chief: $(OBJS)
	$(CC) $(CCFLAGS) -o chief $(OBJS)

ChiefTesting: $(TEST_OBJS) $(OBJS)
	$(CC) $(CCFLAGS) -o chief $(OBJS) $<

build/test.o: 
	$(CC) $(CCFLAGS) -o ./build/test.o -c ./test/test.cpp

build/%.o: src/%.cpp
	$(CC) $(CCFLAGS) -o $@ -c $<
