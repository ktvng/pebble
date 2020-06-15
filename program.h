#ifndef __PROGRAM_H
#define __PROGRAM_H

#include "main.h"
#include "diagnostics.h"

/// log all messages at or above this level
extern LogSeverityType LogAtLevel;


const bool c_DEBUG = true;
const bool c_ERROR = true;

extern bool RuntimeMsgFlag;
extern bool CompileMsgFlag;

extern std::vector<SystemMessage> RuntimeMsgBuffer;
extern std::vector<SystemMessage> CompileMsgBuffer;

extern Program* PROGRAM;

void ReportCompileMsg(SystemMessageType type, String message);
void ReportRuntimeMsg(SystemMessageType type, String message);

#endif