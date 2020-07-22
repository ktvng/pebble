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

extern extArg_t ExtendedArg;

extern int ExtensionExp;

extern extArg_t InstructionReg;

/// 1 if jump occured, 0 otherwise
extern int JumpStatusReg;

extern uint8_t CmpReg;
const uint8_t CmpRegDefaultValue = 0x3;

extern Object* LastResultReg;

// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

extern std::vector<void*> MemoryStack;


// ---------------------------------------------------------------------------------------------------------------------
// Entity Arrays
extern std::vector<String> ReferenceNames;
extern std::vector<Object*> ConstPrimitives;

extern std::vector<Object*> RuntimeObjects;
extern std::vector<Reference*> RuntimeReferences;


// ---------------------------------------------------------------------------------------------------------------------
// Call Stack
struct CallFrame
{
    extArg_t ReturnToInstructionId;
    extArg_t MemoryStackStart;
    extArg_t Owner;
    std::vector<Scope> LocalScopeStack;
    Object* LastResult;
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
extern std::vector<ByteCodeInstruction> ByteCodeProgram;


// ---------------------------------------------------------------------------------------------------------------------
// Statics
extern Object GodObject;
extern Object NothingObject;
extern Object SomethingObject;


// ---------------------------------------------------------------------------------------------------------------------
// Methods

void DoByteCodeProgram();
void AddRuntimeObject(Object* obj);
void AddRuntimeReference(Reference* ref);


#endif
