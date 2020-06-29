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

extern std::string ProgramOutput;

extern Program* PROGRAM;


// ---------------------------------------------------------------------------------------------------------------------
// Method declarations

Scope* ScopeConstructor(Scope* inheritedScope);
Block* BlockConstructor();

Operation* ParseLine(TokenList& tokens);
Block* ParseBlock(std::vector<CodeLine>::iterator it, std::vector<CodeLine>::iterator end);
Program* ParseProgram(const std::string filepath);

TokenList LexLine(const std::string& line);

Reference* DoOperation(Scope* scope, Operation* op);
Reference* DoBlock(Block* codeBlock);
void DoProgram(Program& program);

void ReportCompileMsg(SystemMessageType type, String message);
void ReportRuntimeMsg(SystemMessageType type, String message);

#endif