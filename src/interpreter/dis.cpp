#include "dis.h"

#include "bytecode.h"
#include "vm.h"
#include "diagnostics.h"
#include "call.h"

// ---------------------------------------------------------------------------------------------------------------------
// Diagnostics

/// returns a string represntation of an instruction [ins]
String ToString(ByteCodeInstruction& ins)
{
    String str;
    String decodedArg;

    if(ins.Op == IndexOfInstruction(BCI_LoadCallName))
    {
        str += "#BCI_LoadCallName";
        decodedArg = CallNames[ins.Arg];
    }
    else if(ins.Op == IndexOfInstruction(BCI_LoadPrimitive))
    {
        str += "#BCI_LoadPrimitive";
        auto call = ConstPrimitives[ins.Arg];
        if(call->BoundScope != &NothingScope)
        {
            decodedArg = StringValueOf(ConstPrimitives[ins.Arg]);
        }
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
    else if(ins.Op == IndexOfInstruction(BCI_Equals))
    {
        str += "#BCI_Equals";
    }
    else if(ins.Op == IndexOfInstruction(BCI_NotEquals))
    {
        str += "#BCI_NotEquals";
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
    else if(ins.Op == IndexOfInstruction(BCI_BindSection))
    {
        str += "#BCI_BindSection";
    }
    else if(ins.Op == IndexOfInstruction(BCI_BindType))
    {
        str += "#BCI_BindType";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Eval))
    {
        str += "#BCI_Eval";
    }
    else if(ins.Op == IndexOfInstruction(BCI_EvalHere))
    {
        str += "#BCI_EvalHere";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Return))
    {
        str += "#BCI_Return";
    }
    else if(ins.Op == IndexOfInstruction(BCI_Array))
    {
        str += "#BCI_Array";
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
    else if(ins.Op == IndexOfInstruction(BCI_Swap))
    {
        str += "#BCI_Swap";
    }
    else if(ins.Op == IndexOfInstruction(BCI_JumpNothing))
    {
        str += "#BCI_JumpNothing";
    }
    else if(ins.Op == IndexOfInstruction(BCI_DropTOS))
    {
        str += "#BCI_DropTOS";
    }
    else
    {
        str += "#?????????" + std::to_string(ins.Op);
    }
    
    while(str.size() < 22)
    {
        str += " ";
    }

    str += "\t" + std::to_string(ins.Arg) + "\t" + decodedArg;
    return str;
}

/// returns a string representation of a [bciProgram] which is a list of ByteCodeInstructions
String ToString(std::vector<ByteCodeInstruction>& bciProgram)
{
    String str = "";
    int i=0;
    for(auto& ins: bciProgram)
    {
        str += std::to_string(i++) + "\t" + ToString(ins)  + "\n";
    }

    return str;
}

/// logs the references and instructions of ByteCodeProgram
void LogProgramInstructions()
{
    String refNames = "Call list\n";
    for(size_t i=0; i<CallNames.size(); i++)
    {
        refNames +=  Msg("%i:\t %s\n", i, CallNames[i]);
    }
    
    LogIt(LogSeverityType::Sev2_Important, "ByteCodeProgram", refNames);

    String primitives = "Const Primitives\n";
    for(size_t i=0; i<ConstPrimitives.size(); i++)
    {
        primitives += Msg("%i: %s", i, ToString(ConstPrimitives[i]));
    }
    
    LogIt(LogSeverityType::Sev2_Important, "ByteCodeProgram", primitives);
    LogIt(LogSeverityType::Sev2_Important, "ByteCodeProgram", Msg("\n%s", ToString(ByteCodeProgram)));
}
