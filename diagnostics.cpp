#include <string>
#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <ctime>

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

String itos2(int i)
{
    if(i == 0)
        return "00";
    if(i < 10)
        return MSG("0%i", i);
    return MSG("%i", i);
}

void LogIt(LogSeverityType type, String method, String message)
{
    if(type < LogSeverityLevel)
        return;
    
    time_t rawTime;
    struct tm * timeInfo;

    std::time(&rawTime);
    timeInfo = std::localtime(&rawTime);


    String timeString = MSG("[%i/%s/%s %s:%s:%s]", 
        (1900 + timeInfo->tm_year), 
        itos2(timeInfo->tm_mon), 
        itos2(timeInfo->tm_mday), 
        itos2(timeInfo->tm_hour), 
        itos2(timeInfo->tm_min), 
        itos2(timeInfo->tm_sec));

    std::ofstream logfile;
    logfile.open(".\\logs\\log");
    logfile << timeString << MSG("[%s]", LogSeverityTypeString.at(type)) << MSG("[%s]: ", method) << message << std::endl;
    logfile.close();
}


String SystemMessageTypeString(SystemMessageType type)
{
    switch(type)
    {
        case SystemMessageType::Exception:
        return "Exception";

        case SystemMessageType::Warning:
        return "Warning";

        case SystemMessageType::Advice:
        return "Advice";

        default:
        LogIt(LogSeverityType::Sev1_Notify, "SystemMessageTypeString", "unimplemented SystemMessageType");
        return "";
    }
}

void ReportMsgInternal(std::vector<SystemMessage>& msgBuffer, int lineNumber)
{
    if(!c_ERROR)
        return;

    for(SystemMessage msg: msgBuffer)
    {
        std::cerr << MSG("(!) %s at line[%i]: ", SystemMessageTypeString(msg.Type), lineNumber)  << msg.Content << "\n";
    }
    msgBuffer.clear();
}

void RuntimeMsgPrint(int lineNumber)
{
    ReportMsgInternal(RuntimeMsgBuffer, lineNumber);
}

void CompileMsgPrint(int lineNumber)
{
    ReportMsgInternal(CompileMsgBuffer, lineNumber);
}

String MSG(String message, ...)
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