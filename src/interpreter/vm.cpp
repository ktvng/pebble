#include "vm.h"

#include "bytecode.h"
#include "errormsg.h"

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
        LogItDebug(ToString(ins));
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


// ---------------------------------------------------------------------------------------------------------------------
// Diagnostics

/// returns a string represntation of an instruction [ins]
String ToString(ByteCodeInstruction& ins)
{
    String str;
    String decodedArg;

    if(ins.Op == IndexOfInstruction(BCI_LoadRefName))
    {
        str += "#BCI_LoadRefName";
        decodedArg = ReferenceNames[ins.Arg];
    }
    else if(ins.Op == IndexOfInstruction(BCI_LoadPrimitive))
    {
        str += "#BCI_LoadPrimitive";
        decodedArg = GetStringValue(*ConstPrimitives[ins.Arg]);
    }
    else if(ins.Op == IndexOfInstruction(BCI_Dereference))
    {
        str += "#BCI_Dereference";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Assign)) 
    {
        str += "#BCI_Assign";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Add))
    {
        str += "#BCI_Add";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Subtract))
    {
        str += "#BCI_Subtract";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Multiply))
    {
        str += "#BCI_Multiply";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Divide))
    {
        str += "#BCI_Divide";
    }
    else if(ins.Op == IndexOfInstruction(BCI_SysCall))
    {
        str += "#BCI_SysCall";
    }
    else if(ins.Op == IndexOfInstruction(BCI_And))
    {
        str += "#BCI_And";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Or))
    {
        str += "#BCI_Or";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Not))
    {
        str += "#BCI_Not";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Cmp))
    {
        str += "#BCI_Cmp";
    }
    else if(ins.Op == IndexOfInstruction(BCI_LoadCmp))
    {
        str += "#BCI_LoadCmp";
    }
    else if(ins.Op == IndexOfInstruction(BCI_JumpFalse))
    {
        str += "#BCI_JumpFalse";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Jump))
    {
        str += "#BCI_Jump";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Copy))
    {
        str += "#BCI_Copy";
    }
    else if(ins.Op == IndexOfInstruction(BCI_ResolveDirect))
    {
        str += "#BCI_ResolveDirect";
    }
    else if(ins.Op == IndexOfInstruction(BCI_ResolveScoped))
    {
        str += "#BCI_ResolveScoped";
    }
    else if(ins.Op == IndexOfInstruction(BCI_DefMethod))
    {
        str += "#BCI_DefMethod";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Eval))
    {
        str += "#BCI_Eval";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Return))
    {
        str += "#BCI_Return";
    }
    else if(ins.Op == IndexOfInstruction(BCI_EnterLocal))
    {
        str += "#BCI_EnterLocal";
    }
    else if(ins.Op == IndexOfInstruction(BCI_LeaveLocal))
    {
        str += "#BCI_LeaveLocal";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Extend))
    {
        str += "#BCI_Extend";
    }
    else if(ins.Op == IndexOfInstruction(BCI_NOP))
    {
        str += "#BCI_NOP";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Dup))
    {
        str += "#BCI_Dup";
    }
    else if(ins.Op == IndexOfInstruction(BCI_EndLine))
    {
        str += "#BCI_EndLine";
    }
    else
    {
        str += "#?????????" + std::to_string(ins.Op);
    }
    
    while(str.size() < 22)
    {
        str += " ";
    }

    str += "\t" + std::to_string(ins.Arg) + "\t" + decodedArg + "\n";
    return str;
}

/// returns a string representation of a [bciProgram] which is a list of ByteCodeInstructions
String ToString(std::vector<ByteCodeInstruction>& bciProgram)
{
    String str = "";
    int i=0;
    for(auto& ins: bciProgram)
    {
        str += std::to_string(i++) + "\t" + ToString(ins);
    }

    return str;
}

/// logs the references and instructions of ByteCodeProgram
void LogProgramInstructions()
{
    String refNames = "References list\n";
    for(size_t i=0; i<ReferenceNames.size(); i++)
    {
        refNames +=  Msg("%i:\t %s\n", i, ReferenceNames[i]);
    }
    LogIt(LogSeverityType::Sev2_Important, "ByteCodeProgram", refNames);
    LogIt(LogSeverityType::Sev2_Important, "ByteCodeProgram", Msg("\n%s", ToString(ByteCodeProgram)));
}
