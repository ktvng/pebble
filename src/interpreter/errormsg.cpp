#include <iostream>

#include "errormsg.h"

#include "vm.h"
#include "diagnostics.h"
#include "program.h"
#include "main.h"

// ---------------------------------------------------------------------------------------------------------------------
// Error Handling

bool ErrorFlag = false;
int ErrorCode = 0;
String ErrorMsg;
SystemMessageType ErrorType;

std::vector<extArg_t> ByteCodeLineAssociation;

inline int GetLineNumberFromInstructionNumber(extArg_t instructionNumber)
{
    extArg_t i = 0;
    for(; i< ByteCodeLineAssociation.size() && ByteCodeLineAssociation[i] < instructionNumber; i++);
    return i;
}

void IfNeededDisplayError()
{
    if(ErrorFlag)
    {
        ErrorFlag = false;
        int lineNumber = GetLineNumberFromInstructionNumber(InstructionReg);
        String fatalStatus = (ErrorType == SystemMessageType::Exception ? "Fatal" : "");
        auto stringMsg = Msg("(!) %s %s at line[%i]: %s\n     >>   %s\n", fatalStatus, SystemMessageTypeString(ErrorType), lineNumber, ErrorClasses[ErrorCode].ErrorMsg, ErrorMsg);
        SetConsoleColorForMessage(ErrorType);
        if(g_outputOn)
            std::cerr << stringMsg;
        SetConsoleColor(ConsoleColor::White);
        ProgramMsgs.append(stringMsg);
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// Error Classes

ByteCodeError ErrorClasses[] =
{
    {
        0,
        "type mismatch"
    },
    {
        1,
        "unattributable object"
    }
};