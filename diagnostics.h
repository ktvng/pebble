#ifndef __DIAGNOSTICS_H
#define __DIAGNOSTICS_H

#include <string>

#include "arch.h"


void DebugPrint(const std::string& value);
void ErrorPrint(int lineNumber, std::stringstream& errorBuffer);

void PrintDiagnostics(const Object& obj);
void PrintDiagnostics(const Reference& ref);
void PrintDiagnostics(const Operation& op, int level=0);
void PrintDiagnostics(const TokenList& tokenList);

#endif