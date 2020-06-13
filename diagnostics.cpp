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

void RuntimeErrorPrint(int lineNumber)
{
    if(!c_ERROR)
        return;

    std::string line;
    while(std::getline(ErrorBuffer, line))
    {
        std::cerr << "(!) Exception at line[" << lineNumber << "]: "  << line << "\n";
    }
}

String Message(String message, ...)
{
    va_list vl;
    va_start(vl, message);

    String expandedMessage = "";

    // expands message
    bool expandFlag = false;
    for(size_t i=0; i<message.size(); i++)
    {
        if(message.at(i) == '\\')
        {
            expandedMessage += message.at(i++);
            continue;
        }
        if(message.at(i) == '%')
        {
            expandFlag = true;
            continue;
        }
        if(expandFlag)
        {
            expandFlag = false;
            switch(message.at(i))
            {
                case 'i':
                expandedMessage += std::to_string(va_arg(vl, int));
                break;

                case 's':
                expandedMessage += va_arg(vl, String);
                break;

                case 'd':
                expandedMessage += std::to_string(va_arg(vl, double));
                break;

                default:
                break;
            }
            continue;
        }
        expandedMessage += message.at(i);
    }
    return expandedMessage;
}



// Diagnostic printing
void PrintDiagnostics(const Object& obj)
{
    std::cout << "| Class: " << obj.Class << "\n| Value: " << GetStringValue(obj)  << "\n"; 
}

void PrintDiagnostics(const Object* obj)
{
    PrintDiagnostics(*obj);
}

void PrintDiagnostics(const Reference& ref)
{
    std::cout << "| Name: " << ref.Name << "\n";
    PrintDiagnostics(*ref.ToObject);
    std::cout << "\n";
}

void PrintDiagnostics(const Reference* ref)
{
    PrintDiagnostics(*ref);
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

void PrintDiagnostics(const Operation* op, int level)
{
    PrintDiagnostics(*op);
}

void PrintDiagnostics(const Token& token)
{
    std::cout << "| Type: " << GetStringTokenType(token.Type) << "\t Content: " << token.Content << "\n";
}

void PrintDiagnostics(const Token* token)
{
    PrintDiagnostics(*token);
}

void PrintDiagnostics(const TokenList& tokenList)
{
    std::cout << "TOKENS---\n";
    for(const Token* t: tokenList)
    {
        PrintDiagnostics(t);
    }
}

void PrintDiagnostics(const TokenList* tokenList)
{
    PrintDiagnostics(*tokenList);
}