#include "program.h"

Program* PROGRAM;

// Logging
LogSeverityType LogAtLevel = LogSeverityType::Sev0_Debug;

// Error reporting
std::vector<SystemMessage> RuntimeMsgBuffer;
std::vector<SystemMessage> CompileMsgBuffer;

/// flag which is true whenever the RuntimeMsgBuffer contains a pending error message
bool RuntimeMsgFlag = false;
bool CompileMsgFlag = false;

/// adds an error to the error buffer which will be printed when RuntimeErrorPrint is called
void ReportRuntimeMsg(SystemMessageType type, String message)
{
    SystemMessage msg { message, type };
    RuntimeMsgBuffer.push_back(msg);
    RuntimeMsgFlag = true;
}

void ReportCompileMsg(SystemMessageType type, String message)
{
    SystemMessage msg { message, type };
    CompileMsgBuffer.push_back(msg);
    CompileMsgFlag = true;
}