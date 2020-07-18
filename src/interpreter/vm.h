#ifndef __VM_H
#define __VM_H

#include "abstract.h"



// ---------------------------------------------------------------------------------------------------------------------
// Scope Registers

/// stores the program global scope
extern Scope* ProgramReg;

/// stores the caller scope
extern Object* CallerReg;

/// stores the topmost callframe scope
extern Object* SelfReg;

/// stores the local (possibly anonymous) scope
/// in general this is equivalent to SelfReg
extern Scope* LocalScopeReg;


// ---------------------------------------------------------------------------------------------------------------------
// Special Registers
extern String* ReferenceNameReg;

extern extArg_t ExtendedArg;

extern int ExtensionExp;

extern extArg_t InstructionReg;

/// 1 if jump occured, 0 otherwise
extern int JumpStatusReg;

extern uint8_t CmpReg;
const uint8_t CmpRegDefaultValue = 0x3;


// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

extern std::vector<void*> MemoryStack;


// ---------------------------------------------------------------------------------------------------------------------
// Entity Arrays
extern std::vector<String> ReferenceNames;
extern std::vector<Object*> ConstPrimitives;


typedef void (*BCISystemCall)();
extern std::vector<BCISystemCall> SystemFunctions;

// ---------------------------------------------------------------------------------------------------------------------
// Call Stack
struct CallFrame
{
    extArg_t ReturnToInstructionId;
    extArg_t MemoryStackStart;
    int Owner;
    std::vector<Scope> LocalScopeStack;
};

extern std::vector<CallFrame> CallStack;

// ---------------------------------------------------------------------------------------------------------------------
// Anonymous scope stack
// used for scopes that don't have owners (if/while/indent-block) which don't affect
// self or caller
inline std::vector<Scope>& LocalScopeStack()
{
    return CallStack.back().LocalScopeStack;
}

// ---------------------------------------------------------------------------------------------------------------------
// Bytecode program
struct ByteCodeInstruction
{
    uint8_t Op;
    uint8_t Arg;
};

extern std::vector<ByteCodeInstruction> ByteCodeProgram;


// ---------------------------------------------------------------------------------------------------------------------
// Statics
extern Object GodObject;
extern Object NothingObject;
extern Scope NothingScope;
extern Reference NothingReference;
extern Object SomethingObject;
extern Scope SomethingScope;


// ---------------------------------------------------------------------------------------------------------------------
// Methods

void DoByteCodeProgram();
void LogProgramInstructions();
String ToString(ByteCodeInstruction& ins);

#endif
