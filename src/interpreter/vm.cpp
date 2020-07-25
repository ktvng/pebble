#include "vm.h"

#include "bytecode.h"
#include "errormsg.h"
#include "dis.h"

#include "object.h"
#include "scope.h"
#include "reference.h"
#include "diagnostics.h"
#include "program.h"
#include "main.h"
#include "program.h"

// ---------------------------------------------------------------------------------------------------------------------
// Scope Registers

/// stores the program global scope
Scope* ProgramReg = nullptr;

/// stores the caller object
Object* CallerReg = nullptr;

/// stores the self object;
Object* SelfReg = nullptr;

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
Object* LastResultReg = nullptr;


// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

/// stores objects, references, string pointers, etc. which are used by instructions
std::vector<void*> MemoryStack;


// ---------------------------------------------------------------------------------------------------------------------
// Entity Arrays

/// list of all reference names appearing in a program
std::vector<String> ReferenceNames;

/// list of all constant primitives appearing in a program
std::vector<Object*> ConstPrimitives;

/// list of all objects that are created during runtime which are not ConstPrimitives
std::vector<Object*> RuntimeObjects;

/// list of all references that are created during runtime
std::vector<Reference*> RuntimeReferences;

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

/// the "Object" of Pebble programs
Object GodObject
{
    BaseClass,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

/// the "Nothing" object
Object NothingObject
{
    NullClass,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

/// the "Something" object
Object SomethingObject
{
    SomethingClass,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};


// ---------------------------------------------------------------------------------------------------------------------
// Program Execution

/// initializes all registers and pushes the program CallFrame onto the CallStack
void InitRuntime()
{
    GodObject.Attributes = ScopeConstructor(nullptr);
    NothingObject.Attributes = ScopeConstructor(nullptr);
    SomethingObject.Attributes = ScopeConstructor(nullptr);

    std::vector<Scope> localScopeStack;
    extArg_t programEnd = ByteCodeProgram.size();

    RuntimeObjects.clear();
    RuntimeObjects.reserve(256);

    RuntimeReferences.clear();
    RuntimeReferences.reserve(256);

    CallStack.clear();
    CallStack.reserve(256);
    
    /// TODO: update arg 3 (caller ID);
    CallStack.push_back( {programEnd, 0, 0, localScopeStack });

    CallerReg = &NothingObject;
    SelfReg = &NothingObject;

    MemoryStack.clear();
    MemoryStack.reserve(256);

    MemoryStack.push_back(CallerReg);
    MemoryStack.push_back(SelfReg);

    ProgramReg = ScopeConstructor(nullptr);
    LocalScopeReg = ProgramReg;

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

/// TODO: implement
void GracefullyExit()
{
    for(auto ref: RuntimeReferences)
    {
        ReferenceDestructor(ref);
    }

    for(auto obj: RuntimeObjects)
    {
        ObjectDestructor(obj);
    }

    /// start at 3 to ignore Something, Nothing, and Object entities
    for(size_t i=3; i< ConstPrimitives.size(); i++)
    {
        ObjectDestructor(ConstPrimitives[i]);
    }
    
    ScopeDestructor(ProgramReg);
    ScopeDestructor(SomethingObject.Attributes);
    ScopeDestructor(NothingObject.Attributes);
    ScopeDestructor(GodObject.Attributes);
}

/// iterates and executes the instructions stored in BytecodeProgram
void DoByteCodeProgram()
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

        IfNeededDisplayError();
        if(FatalErrorOccured)
        {
            GracefullyExit();
            return;
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
}

/// add [obj] to RuntimeObjects
void AddRuntimeObject(Object* obj)
{
    RuntimeObjects.push_back(obj);
}

/// add [ref] to RuntimeReferences
void AddRuntimeReference(Reference* ref)
{
    RuntimeReferences.push_back(ref);
}

bool ValuesMatch(ObjectClass cls, void* value, Object* obj)
{
    if(cls != obj->Class)
    {
        return false;
    }

    if(cls == StringClass)
    {
        return *static_cast<String*>(value) == *static_cast<String*>(obj->Value);
    }
    else if(cls == IntegerClass)
    {
        return *static_cast<int*>(value) == *static_cast<int*>(obj->Value); 
    }
    else if(cls == DecimalClass)
    {
        return *static_cast<double*>(value) == *static_cast<double*>(obj->Value); 
    }
    else if(cls == BooleanClass)
    {
        return *static_cast<bool*>(value) == *static_cast<bool*>(obj->Value); 
    }

    return false;    
}

Object* FindExistingObject(ObjectClass cls, void* value)
{
    for(auto obj: ConstPrimitives)
    {
        if(ValuesMatch(cls, value, obj))
        {
            return obj;
        }
    }

    for(auto obj: RuntimeObjects)
    {
        if(IsPrimitiveObject(obj) && ValuesMatch(cls, value, obj))
        {
            return obj;
        }
    }

    return nullptr;
}