#ifndef __DIAGNOSTICS_H
#define __DIAGNOSTICS_H

#include <string>

#include "arch.h"

extern std::stringstream ErrorBuffer;

void DebugPrint(const std::string& value);
void ErrorPrint(int lineNumber);

void PrintDiagnostics(const Object& obj);
void PrintDiagnostics(const Reference& ref);
void PrintDiagnostics(const Operation& op, int level=0);
void PrintDiagnostics(const TokenList& tokenList);

#endif