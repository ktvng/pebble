#ifndef __ERRORMSG_H
#define __ERRORMSG_H

#include "abstract.h"

// ---------------------------------------------------------------------------------------------------------------------
// Error Handling

extern bool ErrorFlag;
extern int ErrorCode;
extern String ErrorMsg;
extern SystemMessageType ErrorType;
extern bool FatalErrorOccured;

inline void ReportError(SystemMessageType errorType, int errorCode, String errorMsg)
{
    ErrorFlag = true;

    ErrorMsg = errorMsg;
    ErrorCode = errorCode;
    ErrorType = errorType;
}

inline void ReportFatalError(SystemMessageType errorType, int errorCode, String errorMsg)
{
    ErrorFlag = true;
    FatalErrorOccured = true;

    ErrorMsg = errorMsg;
    ErrorCode = errorCode;
    ErrorType = errorType;
}

struct ByteCodeError
{
    int ErrorCode;
    String ErrorMsg;
};

extern ByteCodeError ErrorClasses[];
void IfNeededDisplayError();

/// stores the running total number of byte code instructions obtained from
/// all lines less that the value of the index
extern std::vector<extArg_t> ByteCodeLineAssociation;

#endif