#ifndef __DIAGNOSTICS_H
#define __DIAGNOSTICS_H

#include <string>
#include <cstdarg>

#include "arch.h"



enum class SystemMessageType
{
    Exception,
    Warning,
    Advice
};

struct SystemMessage
{
    String Content;
    SystemMessageType Type;
};

extern std::vector<SystemMessage> RuntimeMsgBuffer;
extern std::vector<SystemMessage> CompileMsgBuffer;

void LogIt(LogSeverityType type, String method, String message);
void RuntimeMsgPrint(int lineNumber);
void CompileMsgPrint(int lineNumber);

/// creates a String by expanding a [message] and its variable arguments
String MSG(String message, ...);


void PrintDiagnostics(const Object& obj);
void PrintDiagnostics(const Reference& ref);
void PrintDiagnostics(const Operation& op, int level=0);
void PrintDiagnostics(const TokenList& tokenList);
void PrintDiagnostics(const Token& token);

void PrintDiagnostics(const Object* obj);
void PrintDiagnostics(const Reference* ref);
void PrintDiagnostics(const Operation* op, int level=0);
void PrintDiagnostics(const TokenList* tokenList);
void PrintDiagnostics(const Token* token);

#endif