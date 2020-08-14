#include <iostream>

#include "errormsg.h"

#include "vm.h"
#include "diagnostics.h"
#include "program.h"
#include "main.h"
#include "program.h"

// ---------------------------------------------------------------------------------------------------------------------
// Error Handling

bool ErrorFlag = false;
int ErrorCode = 0;
String ErrorMsg;
SystemMessageType ErrorType;
bool FatalErrorOccured = false;

std::vector<extArg_t> ByteCodeLineAssociation;

inline int GetLineNumberFromInstructionNumber(extArg_t instructionNumber, Program* p)
{
    extArg_t i = 0;
    for(; i< ByteCodeLineAssociation.size() && ByteCodeLineAssociation[i] < instructionNumber; i++);
    return p->Lines[i-1].LineNumber;
}

void IfNeededDisplayError(Program* p)
{
    if(ErrorFlag)
    {
        ErrorFlag = false;
        int lineNumber = GetLineNumberFromInstructionNumber(InstructionReg, p);        
        String fatalStatus = (ErrorType == SystemMessageType::Exception ? "Fatal" : "");        
        auto stringMsg = Msg("(!) %s %s at line[%i]: %s\n     >>   %s\n", fatalStatus, SystemMessageTypeString(ErrorType), lineNumber, ErrorClasses[ErrorCode].ErrorMsg, ErrorMsg);

        ProgramOutput += stringMsg;
        if(g_outputOn)
            std::cerr << ConsoleColorForMessage(ErrorType) << stringMsg << CONSOLE_RESET;
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
    },
    {
        2,
        "method call arguments mismatch"
    },
    {
        3,
        "variable is not callable"
    },
    {
        4,
        "variable is not indexable"
    },
    {
        5,
        "array index must be Integer typed"
    },
    {
        6,
        "array index out of bounds"
    },
    {
        7,
        "unassignable left operand"
    }
};