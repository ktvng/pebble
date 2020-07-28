SOURCE_DIR=./src
BUILD_DIR=./build
CCFLAGS=-g -pedantic -ansi -Wall -std=c++17
INCLUDE_PATHS=-I./src/ -I./src/parser/ -I./src/interpreter -I./src/walker
TEST_INCLUDE_PATH=-I./test/
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
	./testbuilder.exe $<
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o $@ -c $(word 3, $^)



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
	./testbuilder.exe $<
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o $@ -c $(word 3, $^)


################################################################################
# INTERPRETER 
################################################################################
INTERPRETER_SRCS=$(wildcard ./src/interpreter/*.cpp)
INTERPRETER_OBJS=$(INTERPRETER_SRCS:./src/interpreter/%.cpp=./build/%.o)

# STANDARD BUILD .O
build/%.o: src/interpreter/%.cpp src/interpreter/%.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) -o $@ -c $<

# TEST BUILD .O 
build/%.o.t: src/interpreter/%.cpp src/interpreter/%.h test/src/%.cpp
	./testbuilder.exe $<
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o $@ -c $(word 3, $^)


################################################################################
# AST WALKER 
################################################################################
WALKER_SRCS=$(wildcard ./src/walker/*.cpp)
WALKER_OBJS=$(WALKER_SRCS:./src/walker/%.cpp=./build/%.o)

# STANDARD BUILD .O
build/%.o: src/walker/%.cpp src/walker/%.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) -o $@ -c $<

# TEST BUILD .O 
build/%.o.t: src/walker/%.cpp src/walker/%.h test/src/%.cpp
	./testbuilder.exe $<
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o $@ -c $(word 3, $^)



################################################################################
# PEBBLE MAIN BUILD 
################################################################################
pebble.exe: $(OBJS) $(PARSER_OBJS) $(INTERPRETER_OBJS) $(WALKER_OBJS)
	$(CC) $(CCFLAGS) -o pebble.exe $^




################################################################################
# TESTBUILD 
################################################################################
TEST_SRCS=$(wildcard ./test/src/*.cpp)
TEST_OBJS=$(TEST_SRCS:./test/src/%.cpp=./build/%.o.t)

TestBuilder: ./test/testbuilder.cpp
	$(CC) $(CCFLAGS) -o testbuilder.exe $<
	./testbuilder.exe $(PARSER_SRCS) $(SRCS) $(INTERPRETER_SRCS) $(WALKER_SRCS)

build/test.o: test/test.cpp test/test.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o ./build/test.o -c ./test/test.cpp

build/unittests.o: test/unittests.cpp test/unittests.h test/test.h
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o ./build/unittests.o -c ./test/unittests.cpp

pebble_testbuild.exe: $(TEST_OBJS) build/test.o build/unittests.o
	$(CC) $(CCFLAGS) $(INCLUDE_PATHS) $(TEST_INCLUDE_PATH) -o pebble_testbuild.exe $^
