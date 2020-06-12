#include <string>
#include <iostream>
#include <sstream>
#include <cstdarg>

#include "main.h"
#include "arch.h"
#include "token.h"
#include "object.h"

#include "diagnostics.h"

// Error printing
void DebugPrint(const std::string& value)
{
    if(c_DEBUG)
        std::cout << value << "\n";
}

void ErrorPrint(int lineNumber)
{
    if(!c_ERROR)
        return;

    std::string line;
    while(std::getline(ErrorBuffer, line))
    {
        std::cerr << "(!) Exception at line[" << lineNumber << "]: "  << line << "\n";
    }
}


// Diagnostic printing
void PrintDiagnostics(const Object& obj)
{
    std::cout << "| Class: " << obj.Class << "\n| Value: " << GetStringValue(obj)  << "\n"; 
}

void PrintDiagnostics(const Reference& ref)
{
    std::cout << "| Name: " << ref.Name << "\n";
    PrintDiagnostics(*ref.ToObject);
    std::cout << "\n";
}

void PrintDiagnostics(const Operation& op, int level)
{
    std::string type;
    switch(op.Type){
        case OperationType::Add:
        type = "Add";
        break;

        case OperationType::Return:
        type = "Return";
        break;

        case OperationType::Print:
        type = "Print";
        break;

        case OperationType::Assign:
        type = "Assign";
        break;

        case OperationType::Define:
        type = "Define";
        break;

        default:
        type = "unimplemented";
        break;
    }
    std::cout << "OP---" << level << "\nType " << type << "\n";
    if(op.Type == OperationType::Return)
    {
        PrintDiagnostics(*op.Value);
    }
    for(Operation* operand: op.Operands)
    {
        PrintDiagnostics(*operand, level+1);
    }
}

void PrintDiagnostics(const TokenList& tokenList)
{
    std::cout << "TOKENS---\n";
    for(Token t: tokenList)
    {
        std::cout << "| Type: " << GetStringTokenType(t.Type) << "\t Content: " << t.Content << "\n";
    }
}
