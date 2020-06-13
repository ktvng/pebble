#ifndef __DIAGNOSTICS_H
#define __DIAGNOSTICS_H

#include <string>
#include <cstdarg>

#include "arch.h"

extern std::stringstream ErrorBuffer;

void DebugPrint(const std::string& value);
void RuntimeErrorPrint(int lineNumber);

/// creates a String by expanding a [message] and its variable arguments
String Message(String message, ...);


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