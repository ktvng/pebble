#ifndef __VM_H
#define __VM_H

#include "abstract.h"



// ---------------------------------------------------------------------------------------------------------------------
// Scope Registers

/// stores the program global scope
extern Scope* ProgramReg;

/// stores the caller scope
extern Call* CallerReg;

/// stores the topmost callframe scope
extern Call* SelfReg;

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

extern Call* LastResultReg;

// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

extern std::vector<void*> MemoryStack;


// ---------------------------------------------------------------------------------------------------------------------
// Entity Arrays
extern std::vector<String> CallNames;
extern std::vector<Call*> ConstPrimitives;

extern std::vector<Call*> RuntimeCalls;
extern std::vector<Scope*> RuntimeScopes;

// ---------------------------------------------------------------------------------------------------------------------
// Call Stack
struct CallFrame
{
    extArg_t ReturnToInstructionId;
    extArg_t MemoryStackStart;
    extArg_t Owner;
    std::vector<Scope> LocalScopeStack;
    Call* LastResult;
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

extern Call NothingCall;
extern Call ObjectCall;
extern Call SomethingCall;
extern Call ArrayCall;
extern Call IntegerCall;
extern Call DecimalCall;
extern Call StringCall;
extern Call BooleanCall;

extern Scope NothingScope;
extern Scope SomethingScope;
extern const String NothingCallName;

extern const String SizeCallName;
String* CallNamePointerFor(int i);
void IfNeededAddArrayIndexCalls(size_t n);

// ---------------------------------------------------------------------------------------------------------------------
// Methods

int DoByteCodeProgram(Program* p);
void AddRuntimeCall(Call* call);
void AddRuntimeScope(Scope* scope);

#endif
