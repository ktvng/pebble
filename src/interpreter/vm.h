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

/// stores the extended argument if necessary
extern extArg_t ExtendedArg;

/// stores the exponent on the highest order byte of ExtendedArg
extern int ExtensionExp;

/// stores the current instruction number
extern extArg_t InstructionReg;

/// 1 if jump occured, 0 otherwise
extern int JumpStatusReg;

/// stores the result of perfoming a BCI_Cmp operation
/// bit at position [i] indicates:
/// 0: hardcoded false
/// 1: hardcoded true
/// 2: ==
/// 3: <
/// 4: >
/// 5: <=
/// 6: >=
extern uint8_t CmpReg;

/// default value for the CmpReg to maintain the priors
constexpr uint8_t CmpRegDefaultValue = 0x3;

/// stores a pointer the call returned by the last line of code
extern Call* LastResultReg;



// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

/// a stack which represents the working memory of the vm
extern std::vector<void*> MemoryStack;


// ---------------------------------------------------------------------------------------------------------------------
// Entity Arrays

/// stores the names of all calls defined in a program
extern std::vector<String> CallNames;

/// stores the calls for all primitives defined in a program, including the 
/// built in primitives (String/Integer/Decimal/Boolean/Nothing/Object/Array)
extern std::vector<Call*> ConstPrimitives;

/// stores the calls which are created during runtime
extern std::vector<Call*> RuntimeCalls;

/// stores the scopes which are created during runtime
extern std::vector<Scope*> RuntimeScopes;

// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

/// a CallFrame is used to store information required to execute and then return
/// from a method call
/// [ReturnToInstructionId] will be the intruction id returned to
/// [MemoryStackStart] stores the starting index on the memory stack for all
///                    memory used by the method
/// [Owner] refers to the method 
/// [LocalScopeStack] is a stack of Scopes used to handle local scope changes
/// [LastResult] stores the value of LastResultReg before the method is called
struct CallFrame
{
    extArg_t ReturnToInstructionId;
    extArg_t MemoryStackStart;
    extArg_t Owner;
    std::vector<Scope> LocalScopeStack;
    Call* LastResult;
};

/// the call stack used by the vm
extern std::vector<CallFrame> CallStack;


// ---------------------------------------------------------------------------------------------------------------------
// Bytecode program

/// stores the information for executing a bytecode instruction
/// [Op] is the id of the instruction in BCI_Instructions
/// [Arg] is the argument provided to the instruction
struct ByteCodeInstruction
{
    uint8_t Op;
    uint8_t Arg;
};

/// a list of ByteCodeInstructions which represents a executable program
extern std::vector<ByteCodeInstruction> ByteCodeProgram;


// ---------------------------------------------------------------------------------------------------------------------
// Static primtiives

extern Call NothingCall;
extern Call ObjectCall;
extern Call ArrayCall;
extern Call IntegerCall;
extern Call DecimalCall;
extern Call StringCall;
extern Call BooleanCall;

/// the scope of NothingCall which is also assigned to new Calls and calls 
/// which have yet to be bound to a scope
extern Scope NothingScope;

/// the scope used for Primitives which have values
extern Scope SomethingScope;

/// TODO: fix arrays
extern const String SizeCallName;
String* CallNamePointerFor(int i);
void IfNeededAddArrayIndexCalls(size_t n);

// ---------------------------------------------------------------------------------------------------------------------
// Public interface methods

/// executes the bytecodeprogram representation of [p]
int DoByteCodeProgram(Program* p);

/// add a [call] to the list of runtime calls
void AddRuntimeCall(Call* call);

/// add a [scope] to the list of runtime scopes
void AddRuntimeScope(Scope* scope);

#endif
