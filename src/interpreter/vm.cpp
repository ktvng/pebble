#include "vm.h"

#include "bytecode.h"
#include "errormsg.h"
#include "dis.h"
#include "flattener.h"

#include "object.h"
#include "scope.h"
#include "reference.h"
#include "diagnostics.h"
#include "program.h"
#include "main.h"
#include "program.h"
#include "call.h"
#include "value.h"


// ---------------------------------------------------------------------------------------------------------------------
// Scope Registers

/// stores the program global scope
Scope* ProgramReg = nullptr;

/// stores the caller object
Call* CallerReg = nullptr;

/// stores the self object;
Call* SelfReg = nullptr;

/// stores the local (possibly anonymous) scope
/// in general this is equivalent to the scope of SelfReg, when in the base level of the program
/// this will be the same as ProgramReg
Scope* LocalScopeReg = nullptr;


// ---------------------------------------------------------------------------------------------------------------------
// Special Registers

/// stores the extended argument needed when an argument does not fit inside a single byte
extArg_t ExtendedArg;

/// stores the current number of bytes (exponent) which the ExtendedArg contains
int ExtensionExp;

/// stores the index of the current instruction
extArg_t InstructionReg;

/// 1 if jump occured, 0 otherwise
int JumpStatusReg;

/// stores the result of perfoming a BCI_Cmp operation
/// bit at position [i] indicates:
/// 0: hardcoded false
/// 1: hardcoded true
/// 2: ==
/// 3: <
/// 4: >
/// 5: <=
/// 6: >=
/// 7: true if all comparisons valid, false if only == valid
uint8_t CmpReg = CmpRegDefaultValue;

/// stores the result of the last line of code and is updated by the BCI_Endline 
/// instruction
Call* LastResultReg = nullptr;

bool LocalScopeIsDetachedReg = 0;

// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

/// stores objects, references, string pointers, etc. which are used by instructions
std::vector<void*> MemoryStack;


// ---------------------------------------------------------------------------------------------------------------------
// Entity Arrays

String* SimpleCallNames[] = {
    &ObjectType,
    &NothingType,
    &ArrayType,
    &IntegerType,
    &DecimalType,
    &StringType,
    &BooleanType,
};

/// list of all reference names appearing in a program
std::vector<String> CallNames;

/// list of all constant primitives appearing in a program
std::vector<Call*> ConstPrimitives;

/// list of all calls created during runtime
std::vector<Call*> RuntimeCalls;

/// list of all scopes created during runtime
std::vector<Scope*> RuntimeScopes;


// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

/// the call stack, stores information about different function calls and 
/// how to return after a given function returns
std::vector<CallFrame> CallStack;


// ---------------------------------------------------------------------------------------------------------------------
// Bytecode program

/// a list of instructions to be executed sequentially
std::vector<ByteCodeInstruction> ByteCodeProgram;


// ---------------------------------------------------------------------------------------------------------------------
// Statics

/// scope used for Calls which have not been bound to a speciic scope prior
Scope NothingScope;

/// scope used for primitives which are valued 
Scope SomethingScope;

Call ObjectCall
{
    &ObjectType,
    &AbstractObjectType,
    0,
    &SomethingScope,
};

Call NothingCall
{
    &NothingType,
    &NothingType,
    0,
    &NothingScope,
};

Call ArrayCall
{
    &ArrayType,
    &AbstractArrayType,
    0,
    &SomethingScope,
};

Call IntegerCall
{
    &IntegerType,
    &AbstractIntegerType,
    0,
    &SomethingScope,
};

Call DecimalCall
{
    &DecimalType,
    &AbstractDecimalType,
    0,
    &SomethingScope,
};

Call StringCall
{
    &StringType,
    &AbstractStringType,
    0,
    &SomethingScope,
};

Call BooleanCall
{
    &BooleanType,
    &AbstractBooleanType,
    0,
    &SomethingScope,
};


// ---------------------------------------------------------------------------------------------------------------------
// Array methods
/// TODO: fix arrays

const String SizeCallName = "Size";

std::vector<String> ArrayIndexCalls;

String* CallNamePointerFor(int i)
{
    return &ArrayIndexCalls[i];
}

void IfNeededAddArrayIndexCalls(size_t n)
{
    if(n > ArrayIndexCalls.size())
    {
        for(size_t i=ArrayIndexCalls.size(); i<n; i++)
        {
            ArrayIndexCalls.push_back("?");
        }
    }
}

void AddSimpleCallsToProgramScope()
{
    AddCallToScope(&ObjectCall, ProgramReg);
    AddCallToScope(&NothingCall, ProgramReg);
    AddCallToScope(&ArrayCall, ProgramReg);
    AddCallToScope(&IntegerCall, ProgramReg);
    AddCallToScope(&DecimalCall, ProgramReg);
    AddCallToScope(&StringCall, ProgramReg);
    AddCallToScope(&BooleanCall, ProgramReg);
}

// ---------------------------------------------------------------------------------------------------------------------
// Program execution helpers

/// initializes all registers and pushes the program CallFrame onto the CallStack
void InitRuntime()
{
    std::vector<LabeledScope> localScopeStack;
    extArg_t programEnd = ByteCodeProgram.size();

    RuntimeCalls.clear();
    RuntimeCalls.reserve(256);

    RuntimeCalls = { &ObjectCall, &NothingCall, &ArrayCall, &IntegerCall,
        &DecimalCall, &StringCall, &BooleanCall };

    RuntimeScopes.clear();
    RuntimeScopes.reserve(256);

    CallStack.clear();
    CallStack.reserve(256);
    
    /// TODO: update arg 3 (caller ID);
    CallStack.push_back( {programEnd, 0, 0, localScopeStack });

    CallerReg = &NothingCall;
    SelfReg = &NothingCall;

    MemoryStack.clear();
    MemoryStack.reserve(256);

    MemoryStack.push_back(CallerReg);
    MemoryStack.push_back(SelfReg);

    ProgramReg = ScopeConstructor(nullptr);
    LocalScopeReg = ProgramReg;
    AddSimpleCallsToProgramScope();

    FatalErrorOccured = false;
    ErrorFlag = false;

    InstructionReg = 0;
}

/// true if [ins] is not an Extend instruction and the exponent of the ExtendedArg
/// stored at ExtensionExp is not 0;
inline bool ShouldUseExtendedArg(const ByteCodeInstruction& ins)
{
    return ins.Op != IndexOfInstruction(BCI_Extend) && ExtensionExp;
}

/// true if [ins] is a NOP
inline bool IsNOP(const ByteCodeInstruction& ins)
{
    return ins.Op == IndexOfInstruction(BCI_NOP);
}

// ---------------------------------------------------------------------------------------------------------------------
// Program post-execution cleanup

/// list containing the memory addresses of Call values which have already
/// been destroyed
std::vector<String*> DestroyedValues;

/// true if [value] has already been destroyed and appears in DestroyedValues
/// otherwise will return false and add [value] to the list
bool HasBeenDestroyed(String* value)
{
    if(value == nullptr)
        return true;

    for(auto val: DestroyedValues)
    {
        if(val == value)
        {
            return true;
        }
    }

    DestroyedValues.push_back(value);
    return false;
}

/// true if [call] has a BoundValue that should be destroyed
bool ShouldDestroyValue(const Call* call)
{
    return call->BoundType == &StringType && call->BoundScope != &NothingScope;
}

/// wrapper to delete a [call] and meet all dependencies
void DeleteCall(Call* call)
{
    if(ShouldDestroyValue(call) && !HasBeenDestroyed(call->BoundValue.s))
    {
        StringDestructor(call->BoundValue.s);
    }
    CallDestructor(call);
}

/// free all allocated memory and stop program execution
void GracefullyExit()
{
    size_t size = RuntimeCalls.size() + ConstPrimitives.size();
    DestroyedValues.clear();
    DestroyedValues.reserve(size);
    
    for(size_t i=SIMPLE_CALLS; i<RuntimeCalls.size(); i++)
    {
        DeleteCall(RuntimeCalls[i]);
    }

    for(auto scope: RuntimeScopes)
    {
        ScopeDestructor(scope);
    }

    for(auto call: ConstPrimitives)
    {
        DeleteCall(call);
    }
    ScopeDestructor(ProgramReg);
}


// ---------------------------------------------------------------------------------------------------------------------
// Program Execution

/// iterates and executes the instructions stored in BytecodeProgram
int DoByteCodeProgram(Program* p)
{
    InitRuntime();

    while(InstructionReg < ByteCodeProgram.size())
    {
        auto ins = ByteCodeProgram[InstructionReg];
        LogItDebug(Msg("%i: %s", (int)InstructionReg, ToString(ins)));

        if(IsNOP(ins))
        {
            InstructionReg++;
            continue;
        }

        if(ShouldUseExtendedArg(ins))
        {
            BCI_Instructions[ins.Op](ExtendedArg ^ ins.Arg);
            ExtendedArg = 0;
            ExtensionExp = 0;
        }
        else
        {
            BCI_Instructions[ins.Op](ins.Arg);
        }

        IfNeededDisplayError(p);
        if(FatalErrorOccured)
        {
            GracefullyExit();
            return 1;
        }

        if(!JumpStatusReg)
        {
            InstructionReg++;
        }
        else
        {
            JumpStatusReg = 0;
        }
    }
    
    if(LogAtLevel == LogSeverityType::Sev0_Debug)
    {
        std::cout << "\nmem#" << MemoryStack.size() << "\n";
    }
    GracefullyExit();
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
// Recording created entities

void AddRuntimeCall(Call* call)
{
    RuntimeCalls.push_back(call);
}

void AddRuntimeScope(Scope* scope)
{
    RuntimeScopes.push_back(scope);
}
