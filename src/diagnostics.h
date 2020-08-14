#ifndef __DIAGNOSTICS_H
#define __DIAGNOSTICS_H

#include <string>
#include <cstdarg>

#include "abstract.h"


// ---------------------------------------------------------------------------------------------------------------------
// Logging and debugging


/// converts LogSeverityType to a printable string
const std::map<LogSeverityType, String> LogSeverityTypeString =
{
    { LogSeverityType::Sev3_Critical, "Critical" },
    { LogSeverityType::Sev2_Important, "Important" },
    { LogSeverityType::Sev1_Notify, "Notify" },
    { LogSeverityType::Sev0_Debug, "Debug" }
};


// ---------------------------------------------------------------------------------------------------------------------
// Console

/// console display color
#define CONSOLE_RESET   "\033[0m"
#define CONSOLE_BLACK   "\033[30m"
#define CONSOLE_RED     "\033[31m"
#define CONSOLE_GREEN   "\033[32m"
#define CONSOLE_YELLOW  "\033[33m"
#define CONSOLE_BLUE    "\033[34m"
#define CONSOLE_MAGENTA "\033[35m"
#define CONSOLE_CYAN    "\033[36m"
#define CONSOLE_WHITE   "\033[37m"

const char* ConsoleColorForMessage(SystemMessageType type);

// ---------------------------------------------------------------------------------------------------------------------
// String formatting utilties

/// creates a String by expanding a [message] and its variable arguments
String Msg(String message, ...);

String IndentStringToLevel(String str, int level, int margin=0);


// ---------------------------------------------------------------------------------------------------------------------
// Pebble system messages


extern std::vector<SystemMessage> RuntimeMsgBuffer;
extern std::vector<SystemMessage> CompileMsgBuffer;

void RuntimeMsgPrint(int lineNumber);
void CompileMsgPrint(int lineNumber);

String SystemMessageTypeString(SystemMessageType type);
// ---------------------------------------------------------------------------------------------------------------------
// General logging

void PurgeLog();
void LogIt(LogSeverityType type, String method, String message);
void LogItDebug(String message, String method="unspecified");


// ---------------------------------------------------------------------------------------------------------------------
// ToString methods

String ToString(const OperationType& type);
String ToString(const Operation& op, int level=0);
String ToString(const Operation* op, int level=0);
String ToString(const Block* block, int level=0);
String ToString(const Block& block, int level=0);
String ToString(const Call* call);

// ---------------------------------------------------------------------------------------------------------------------
// Logging object dump

void LogDiagnostics(const Object& obj, String message="object dump", String method="unspecified");
void LogDiagnostics(const Operation& op, String message="object dump", String method="unspecified");
void LogDiagnostics(const TokenList& tokenList, String message="object dump", String method="unspecified");
void LogDiagnostics(const Token& token, String message="object dump", String method="unspecified");
void LogDiagnostics(const ObjectReferenceMap& map, String message="object dump", String method="unspecified");

void LogDiagnostics(const Object* obj, String message="object dump", String method="unspecified");
void LogDiagnostics(const Reference* ref, String message="object dump", String method="unspecified");
void LogDiagnostics(const Operation* op, String message="object dump", String method="unspecified");
void LogDiagnostics(const TokenList* tokenList, String message="object dumpLogItDebug", String method="unspecified");
void LogDiagnostics(const Token* token, String message="object dumpLogItDebug", String method="unspecified");

void LogDiagnostics(const Block* b, String message="object dumpLogItDebug", String method="unspecified");
void LogDiagnostics(const Call* call, String message="object dumpLogItDebug", String method="unspecified");

#endif
