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
enum ConsoleColor
{
    None,
    Blue,
    DarkGreen,
    LightBlue,
    Red,
    Purple,
    Yellow,
    White,
    Grey,
    DarkBlue,
    Green,
    Cyan,
    DarkRed,
    Purple2,
    Yellow2,
    StarkWhite
};

/// changes the color for std::cout
void SetConsoleColor(ConsoleColor color);


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
void LogDiagnostics(const ObjectReferenceMap* map, String message="object dumpLogItDebug", String method="unspecified");

void LogDiagnostics(const Block* b, String message="object dumpLogItDebug", String method="unspecified");

#endif
