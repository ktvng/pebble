#include "vm.h"

#include "bytecode.h"
#include "errormsg.h"

#include "object.h"
#include "scope.h"
#include "reference.h"
#include "diagnostics.h"
#include "program.h"
#include "main.h"

// ---------------------------------------------------------------------------------------------------------------------
// Scope Registers

/// stores the program global scope
Scope* ProgramReg = nullptr;

/// stores the caller scope
Object* CallerReg = nullptr;

/// stores the topmost callframe scope
Object* SelfReg = nullptr;

/// stores the local (possibly anonymous) scope
/// in general this is equivalent to SelfReg
Scope* LocalScopeReg = nullptr;


// ---------------------------------------------------------------------------------------------------------------------
// Special Registers
String* ReferenceNameReg;

extArg_t ExtendedArg;

int ExtensionExp;

extArg_t InstructionReg;

/// 1 if jump occured, 0 otherwise
int JumpStatusReg;

/// bit at position [i] indicates:
/// 0: hardcoded false
/// 1: hardcoded true
/// 2: ==
/// 3: <
/// 4: >
/// 5: <=
/// 6: >=
uint8_t CmpReg = CmpRegDefaultValue;


// ---------------------------------------------------------------------------------------------------------------------
// Call Stack

std::vector<void*> MemoryStack;


// ---------------------------------------------------------------------------------------------------------------------
// Entity Arrays
std::vector<String> ReferenceNames;
std::vector<Object*> ConstPrimitives;


std::vector<BCISystemCall> SystemFunctions;

// ---------------------------------------------------------------------------------------------------------------------
// Call Stack
std::vector<CallFrame> CallStack;

// ---------------------------------------------------------------------------------------------------------------------
// Bytecode program

std::vector<ByteCodeInstruction> ByteCodeProgram;


// ---------------------------------------------------------------------------------------------------------------------
// Statics
Object GodObject
{
    BaseClass,
    ScopeConstructor(nullptr),
    nullptr,
    nullptr,
    nullptr
};


Object NothingObject
{
    NullClass,
    ScopeConstructor(nullptr),
    nullptr,
    nullptr,
    nullptr
};

Scope NothingScope = *NothingObject.Attributes;

Reference NothingReference
{
    "Nothing",
    &NothingObject,
};

Object SomethingObject
{
    SomethingClass,
    ScopeConstructor(nullptr),
    nullptr,
    nullptr,
    nullptr
};

Scope SomethingScope = *SomethingObject.Attributes;



// ---------------------------------------------------------------------------------------------------------------------
// Program Execution


void InitRuntime()
{
    std::vector<Scope> localScopeStack;
    unsigned long programEnd = ByteCodeProgram.size();
    
    /// TODO: update arg 3 (caller ID);
    CallStack.push_back( {programEnd, 0, 0, localScopeStack });

    CallerReg = &NothingObject;
    SelfReg = &NothingObject;

    ProgramReg = ScopeConstructor(nullptr);
    LocalScopeReg = ProgramReg;
}

/// true if ins is not Extend and 
inline bool ShouldUseExtendedArg(const ByteCodeInstruction& ins)
{
    return ins.Op != IndexOfInstruction(BCI_Extend) && ExtensionExp;
}

inline bool IsNOP(const ByteCodeInstruction& ins)
{
    return ins.Op == IndexOfInstruction(BCI_NOP);
}

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

        if(!JumpStatusReg)
        {
            InstructionReg++;
        }
        else
        {
            JumpStatusReg = 0;
        }

    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Diagnostics

String ToString(ByteCodeInstruction& ins)
{
    String str;
    if(ins.Op == IndexOfInstruction(BCI_LoadRefName))
    {
        str += "#BCI_LoadRefName";
    }
    else if(ins.Op == IndexOfInstruction(BCI_LoadPrimitive))
    {
        str += "#BCI_LoadPrimitive";
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
    else
    {
        str += "#????????????";
    }
    str += "\t " + std::to_string(ins.Arg) + "\n";
    return str;
}

String ToString(std::vector<ByteCodeInstruction>& bciProgram)
{
    String str = "";
    int i=0;
    for(auto& ins: bciProgram)
    {
        str += std::to_string(i++) + ToString(ins);
    }

    return str;
}

void LogProgramInstructions()
{
    LogIt(LogSeverityType::Sev2_Important, "", "References");
    for(size_t i=0; i<ReferenceNames.size(); i++)
    {
        LogIt(LogSeverityType::Sev2_Important, "", Msg("%i: %s", i, ReferenceNames[i]));
    }
    LogIt(LogSeverityType::Sev2_Important, "ByteCodeProgram", Msg("\n%s", ToString(ByteCodeProgram)));
}
