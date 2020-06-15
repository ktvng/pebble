#ifndef __DIAGNOSTICS_H
#define __DIAGNOSTICS_H

#include <string>
#include <cstdarg>

#include "main.h"
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

void PurgeLog();
void LogIt(LogSeverityType type, String method, String message);
void LogItDebug(String message, String method="unspecified");
void RuntimeMsgPrint(int lineNumber);
void CompileMsgPrint(int lineNumber);

/// creates a String by expanding a [message] and its variable arguments
String MSG(String message, ...);

void LogDiagnostics(const Object& obj, String message="object dump", String method="unspecified");
void PrintDiagnostics(const Operation& op, int level=0); // TODO: Change to log
void LogDiagnostics(const TokenList& tokenList, String message="object dump", String method="unspecified");
void LogDiagnostics(const Token& token, String message="object dump", String method="unspecified");
void LogDiagnostics(const ObjectReferenceMap& map, String message="object dump", String method="unspecified");

void LogDiagnostics(const Object* obj, String message="object dump", String method="unspecified");
void LogDiagnostics(const Reference* ref, String message="object dump", String method="unspecified");
void PrintDiagnostics(const Operation* op, int level=0);
void LogDiagnostics(const TokenList* tokenList, String message="object dumpLogItDebug", String method="unspecified");
void LogDiagnostics(const Token* token, String message="object dumpLogItDebug", String method="unspecified");
void LogDiagnostics(const ObjectReferenceMap* map, String message="object dumpLogItDebug", String method="unspecified");

#endif
