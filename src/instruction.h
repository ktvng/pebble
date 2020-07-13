#ifndef __INSTRUCTION_H
#define __INSTRUCTION_H


#include "abstract.h"
#include "utils.h"

using namespace utils;

typedef String ReferenceId;
typedef int JumpTo;
/// instructions should pop their operands and the result to the Memory stack
typedef void (*Instruction)(ReferenceId, JumpTo);

struct InstructionCall
{
    Instruction Op;
    ReferenceId Arg1;
    JumpTo Arg2;
};

extern std::vector<InstructionCall> ProgramInstructions;

extern std::vector<Object*> Memory;
extern std::vector<Reference*> ReferencesCache;

extern Reference* ReferenceRegister;

extern int InstructionPointer;

void PrintProgramInstructions();
void FlattenProgram(Program* p);
void DoAllInstructions();
String ToString(InstructionCall& call);

#endif