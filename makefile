SOURCE_DIR=./src
BUILD_DIR=./build
CCFLAGS=-g -pedantic -ansi -Wall -std=c++17
INCLUDE_PATHS=-I./src/ -I./src/parser/
CC=g++


################################################################################
# SRC
################################################################################
SRCS=$(wildcard ./src/*.cpp)
OBJS=$(SRCS:./src/%.cpp=./build/%.o)

# STANDARD BUILD .O
build/%.o: src/%.cpp src/%.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) -o $@ -c $<

# TEST BUILD .O
build/%.o.t: src/%.cpp src/%.h test/src/%.cpp
	./testbuilder $<
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) -o $@ -c $(word 3, $^)



################################################################################
# PARSER 
################################################################################
PARSER_SRCS=$(wildcard ./src/parser/*.cpp)
PARSER_OBJS=$(PARSER_SRCS:./src/parser/%.cpp=./build/%.o)

# STANDARD BUILD .O
build/%.o: src/parser/%.cpp src/parser/%.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) -o $@ -c $<

# TEST BUILD .O 
build/%.o.t: src/parser/%.cpp src/parser/%.h test/src/%.cpp
	./testbuilder $<
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) -o $@ -c $(word 3, $^)



################################################################################
# PEBBLE MAIN BUILD 
################################################################################
pebble.exe: $(OBJS) $(PARSER_OBJS)
	$(CC) $(CCFLAGS) -o pebble $(OBJS) $(PARSER_OBJS)




################################################################################
# TESTBUILD 
################################################################################
TEST_SRCS=$(wildcard ./test/src/*.cpp)
TEST_OBJS=$(TEST_SRCS:./test/src/%.cpp=./build/%.o.t)
TEST_INCLUDE_PATH=-I./test/

TestBuilder: ./test/testbuilder.cpp
	$(CC) $(CCFLAGS) -o testbuilder $<
	./testbuilder.exe $(PARSER_SRCS) $(SRCS)

build/test.o: test/test.cpp test/test.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o ./build/test.o -c ./test/test.cpp

build/unittests.o: test/unittests.cpp test/unittests.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o ./build/unittests.o -c ./test/unittests.cpp

pebble_testbuild.exe: $(TEST_OBJS) build/test.o build/unittests.o
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o pebble_testbuild $^
