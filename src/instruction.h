#ifndef __INSTRUCTION_H
#define __INSTRUCTION_H


#include "abstract.h"
#include "utils.h"

using namespace utils;

struct ByteCodeInstruction;


void PrintProgramInstructions();
void FlattenProgram(Program* p);
void DoByteCodeProgram();

void FlattenBlock(Block* block);
String ToString(ByteCodeInstruction& ins);

#endif