#ifndef __DIAGNOSTICS_CPP
#define __DIAGNOSTICS_CPP

#include <string>
#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>


#include "main.h"
#include "diagnostics.h"

#include "token.h"
#include "object.h"
#include "program.h"
#include "reference.h"
#include "operation.h"
#include "scope.h"


#ifdef _WIN32 
#include <windows.h>
#endif
void SetConsoleColor(ConsoleColor color)
{
#ifdef _WIN32
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
#else
    LogIt(LogSeverityType::Sev1_Notify, "SetConsoleColor", "console color is not supported on linux");
#endif
}

// indent formatting
const String c_indentString = "  ";

String IndentLevel(int level)
{
    if(level <= 0)
        return "";
    
    String levelString;
    for(int i=0; i<level; i++)
    {
        levelString += c_indentString;
    }

    return levelString;
}

String IndentStringToLevel(String str, int level, int margin)
{
    String leveledStr = "";
    for(size_t i=0; i<str.size(); i++)
    {
        leveledStr += str.at(i);
        if(str.at(i) == '\n')
        {
            leveledStr += IndentLevel(level);
            for(int j=0; j<margin; j++)
            {
                leveledStr += " ";
            }
        }
    }
    return leveledStr;
}

const String c_rightEdge = "    || ";

String AddRightEdge(String str)
{
    String formattedStr = c_rightEdge;
    formattedStr.reserve(str.size());
    for(size_t i=0; i<str.size(); i++)
    {
        formattedStr += str.at(i);
        if(str.at(i) == '\n')
            formattedStr += c_rightEdge;
    }
    return formattedStr;
}


// Error printing
String itos2(int i)
{
    if(i < 10)
        return Msg("0%i", i);
    return Msg("%i", i);
}

void PurgeLog()
{
    std::ofstream logfile;
    logfile.open("./logs/log");
    logfile << "";
    logfile.close();
}

void LogIt(LogSeverityType type, String method, String message)
{
    if(type < LogAtLevel)
        return;
    
    time_t rawTime;
    struct tm * timeInfo;

    std::time(&rawTime);
    timeInfo = std::localtime(&rawTime);


    String timeString = Msg("[%i/%s/%s %s:%s:%s]", 
        (1900 + timeInfo->tm_year), 
        itos2(timeInfo->tm_mon), 
        itos2(timeInfo->tm_mday), 
        itos2(timeInfo->tm_hour), 
        itos2(timeInfo->tm_min), 
        itos2(timeInfo->tm_sec));

    std::ofstream logfile;
    logfile.open("./logs/log", std::ios::app);
    logfile << timeString << Msg("[%s]", LogSeverityTypeString.at(type)) << Msg("[%s]: ", method) << message << std::endl;
    logfile.close();
}

void LogItDebug(String message, String method)
{
    LogIt(LogSeverityType::Sev0_Debug, method, message);
}

void DebugDumpObjectToLog(String object, String message, String method="unspecified")
{
    String formattedMessage = message + "\n" + AddRightEdge(object);
    LogIt(LogSeverityType::Sev0_Debug, method, formattedMessage);
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

void SetConsoleColorForMessage(SystemMessageType type)
{
    switch(type)
    {
        case SystemMessageType::Exception:
        SetConsoleColor(ConsoleColor::Red);
        return;

        case SystemMessageType::Warning:
        SetConsoleColor(ConsoleColor::Yellow2);
        return;

        case SystemMessageType::Advice:
        SetConsoleColor(ConsoleColor::Cyan);
        return;
    }
}

void ReportMsgInternal(std::vector<SystemMessage>& msgBuffer, int lineNumber)
{
    if(!c_ERROR)
        return;

    String fatalStatus = (FatalCompileError ? "Fatal" : "" );


    for(SystemMessage msg: msgBuffer)
    {
        auto stringMsg = Msg("(!) %s %s at line[%i]: ", fatalStatus, SystemMessageTypeString(msg.Type), lineNumber) + msg.Content + "\n";
        SetConsoleColorForMessage(msg.Type);
        if(g_outputOn)
            std::cerr << stringMsg;
        SetConsoleColor(ConsoleColor::White);
        ProgramMsgs.append(stringMsg);
        
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

String Msg(String message, ...)
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
            std::stringstream ss;
            String pointerString;
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

                case 'p':
                ss << va_arg(vl, void*);
                ss >> pointerString;
                expandedMessage += pointerString;
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




String StringForAttrbute(String name, String value)
{
    return Msg("-[%s]: %s\n", name, value);
}

int ValueStartOffset(String name)
{
    return 5 + name.size();
}

// if str has effectively two '\n' characters with no text between, return the index of the 
// second newline. -1 otherwise;
int HasDoubledNewLine(String& str, size_t pos)
{
    if(str.at(pos) != '\n')
        return -1;

    for(int i=pos+1; static_cast<size_t>(i)<str.size(); i++)
    {
        if(str.at(i) == '\n')
            return i;
        if(str.at(i) != ' ')
            return -1;
    }
    return -1;
}

String Compact(String str)
{
    String compactString;
    compactString.reserve(str.size());
    for(size_t i=0; i<str.size()-1; i++)
    {
        if(int nextPos = HasDoubledNewLine(str, i); nextPos > 0){
            i = static_cast<size_t>(nextPos);
        }

        compactString+= str.at(i);
    }
    return compactString;
}





template <typename T>
String ToString(std::vector<T> list, String typeString)
{
    if(list.size() == 0)
        return Msg("<List<%s>>", typeString);
    String listString = Msg("<List<%s>>\n", typeString);
    for(size_t i=0; i<list.size(); i++)
    {
        int offset = ValueStartOffset(std::to_string(i));
        listString += IndentLevel(1) +
             StringForAttrbute(std::to_string(i), IndentStringToLevel(ToString(list.at(i)), 1, offset));
    }
    return listString;
}

String ToString(const String& str)
{
    return str;
}

// Diagnostic printing

String ToString(const Method* method)
{ 
    String methodString = Msg("<Method>\n");

    for(auto ref: method->ParameterNames)
    {
        methodString += IndentLevel(1) + 
            StringForAttrbute("param", ref);
    }
    
    if(method->CodeBlock != nullptr)
        methodString += IndentLevel(1) +
            StringForAttrbute("block", IndentStringToLevel(ToString(method->CodeBlock), 1));

    return methodString;
}

String ToString(const Object& obj)
{
    String objString = "<Object>\n";
    
    objString += IndentLevel(1) + 
        StringForAttrbute("Class", obj.Class);

    objString += IndentLevel(1) +
        StringForAttrbute("Value", GetStringValue(&obj));

    objString += IndentLevel(1) +
        StringForAttrbute("Attributes", IndentStringToLevel(ToString(obj.Attributes->ReferencesIndex, "Reference"), 1));

    if(obj.Action != nullptr)
    {
        objString += IndentLevel(1) + 
            StringForAttrbute("Action", IndentStringToLevel(ToString(obj.Action), 1));
    }

    return objString;
}

String ToString(const Object* obj)
{
    return ToString(*obj);
}

String ToString(const Reference* ref)
{
    if(ref->Name == "caller")
        return "caller ";
    if(ref->Name == "self")
        return "self";

    String refString = Msg("<Reference> @ %p\n", ref);

    refString += IndentLevel(1) +
        StringForAttrbute("Name", ref->Name);

    if(ObjectOf(ref) != nullptr)
        refString += IndentLevel(1) +
            StringForAttrbute("ToObject", IndentStringToLevel(ToString(ObjectOf(ref)), 1));

    return refString;
}

String ToString(const ObjectReferenceMap& map)
{
    String mapString = "<ObjectReferenceMap>\n";
    mapString += IndentLevel(1) + 
        StringForAttrbute("Object", IndentStringToLevel(ToString(*map.IndexedObject),1));

    mapString += IndentLevel(1) + StringForAttrbute("Reference", "<List<Reference*>");
    for(size_t i=0; i<map.References.size(); i++)
    {
        mapString += IndentLevel(2) + 
            StringForAttrbute(std::to_string(i), map.References.at(i)->Name 
            + Msg(" @ %p", map.References.at(i)));
    }

    return mapString;
}

String ToString(const Token& token)
{
    return StringForAttrbute("Token", Msg("Position: %i\t Type: %s\t Content: %s", 
        token.Position,
        GetStringTokenType(token.Type),
        token.Content));
}

String ToString(const Token* token)
{
    return ToString(*token);
}

String ToString(const TokenList& tokenList)
{
    return ToString(tokenList, "Token*");
}

String ToString(const TokenList* tokenList)
{
    return ToString(*tokenList);
}

String ToString(const OperationType& type)
{
    // at least in C#, you can cast enums to strings! and get rid of switch
    switch(type){
        case OperationType::Add:
        return "Add";

        case OperationType::Ref:
        return "Ref";

        case OperationType::Print:
        return "Print";

        case OperationType::Assign:
        return "Assign";

        case OperationType::Subtract:
        return "Subtract";
        
        case OperationType::If:
        return "If";

        case OperationType::ElseIf:
        return "ElseIf";

        case OperationType::Else:
        return "Else";

        case OperationType::While:
        return "While";

        case OperationType::And:
        return "And";

        case OperationType::Or:
        return "Or";

        case OperationType::Not:
        return "Not";

        case OperationType::DefineMethod:
        return "DefineMethod";

        case OperationType::Evaluate:
        return "Evaluate";

        case OperationType::EvaluateHere:
        return "EvaluateHere";

        case OperationType::Divide:
        return "Divide";

        case OperationType::Multiply:
        return "Multiply";

        case OperationType::Return:
        return "Return";

        case OperationType::ScopeResolution:
        return "ScopeResolution";

        case OperationType::New:
        return "New";

        case OperationType::Class:
        return "Class";

        case OperationType::Tuple:
        return "Tuple";

        case OperationType::Ask:
        return "Ask";

        case OperationType::IsGreaterThan:
        return "IsGreaterThan";
        
        case OperationType::IsLessThan:
        return "IsLessThan";
        
        case OperationType::IsGreaterThanOrEqualTo:
        return "IsGreaterThanOrEqualTo";
        
        case OperationType::IsLessThanOrEqualTo:
        return "IsLessThanOrEqualTo";
        
        case OperationType::IsEqual:
        return "IsEqual";
        
        case OperationType::IsNotEqual:
        return "IsNotEqual";

        case OperationType::Is:
        return "Is";

        default:
        LogIt(LogSeverityType::Sev2_Important, "ToString", "unimplemented OperationType");
        return "unimplemented";
    }
}

String ToString(const Operation& op, int level)
{
    String opString = "<Operation> L" + std::to_string(level) + "\n";
    opString.reserve(256);

    if(level == 0)
        opString += IndentLevel(1) +
            StringForAttrbute(
                "LineNumber",
                Msg("%i", op.LineNumber));


    if(op.Type == OperationType::Ref)
    {
        if(ObjectOf(op.Value) != nullptr)
            if(ObjectOf(op.Value)->Action != nullptr)
            {
                opString += IndentLevel(1) + 
                    StringForAttrbute("<Reference>", IndentStringToLevel(ToString(ObjectOf(op.Value)), 1));
            }
            else
            {
                opString += IndentLevel(1) + 
                    StringForAttrbute(
                        "OperationType", 
                        Msg("Ref <Reference> %s to %s %s", 
                            op.Value->Name, 
                            ObjectOf(op.Value)->Class,
                            GetStringValue(ObjectOf(op.Value))));
            }

        else
        {
            opString += IndentLevel(1) +
                StringForAttrbute("Name", op.Value->Name);
        }

        return opString;
    }

    opString += IndentLevel(1) + StringForAttrbute(
        "OperationType", ToString(op.Type)
        );

    if(op.Value != nullptr)
    {
        opString += IndentLevel(1) + StringForAttrbute(
            "Value",
            Msg(
                "<Reference> %s to %s %s",
                op.Value->Name, 
                ObjectOf(op.Value)->Class,
                GetStringValue(ObjectOf(op.Value)
                )));
    }

    for(Operation* operand: op.Operands)
    {
        opString += IndentLevel(1) + IndentStringToLevel(
            ToString(operand, level+1),
            1) + "\n";
    }
    
    return opString;
}

String ToString(const Operation* op, int level)
{
    return ToString(*op, level);
}

String ToString(const Block* block, int level)
{
    String blockString = "<Block>\n";
    blockString.reserve(512);

    for(size_t i=0; i<block->Executables.size(); i++)
    {
        Executable* exec = block->Executables.at(i);

        if(exec->ExecType == ExecutableType::Operation)
        {
            blockString += IndentLevel(1) + StringForAttrbute(std::to_string(i),
                IndentStringToLevel(
                    ToString(static_cast<Operation*>(exec)), 
                    1, 
                    ValueStartOffset(std::to_string(i))));
        }
        else if(exec->ExecType == ExecutableType::Block){
            blockString += IndentLevel(1) + StringForAttrbute(std::to_string(i),
                IndentStringToLevel(
                    ToString(static_cast<Block*>(exec)), 
                    1, 
                    ValueStartOffset(std::to_string(i))));
        }
    }

    return blockString;
}





template <typename T>
String DisplayString(T obj)
{
    return Compact(ToString(obj));
}





void LogDiagnostics(const Block* b, String message, String method)
{
    DebugDumpObjectToLog(DisplayString(b), message, method);
}

void LogDiagnostics(const Object& obj, String message, String method)
{
    DebugDumpObjectToLog(DisplayString(obj), message, method);
}

void LogDiagnostics(const Object* obj, String message, String method)
{
    LogDiagnostics(*obj, message, method);
}

void LogDiagnostics(const Reference* ref, String message, String method)
{
    DebugDumpObjectToLog(DisplayString(ref), message, method);
}

void LogDiagnostics(const Operation& op, String message, String method)
{
    DebugDumpObjectToLog(DisplayString(op), message, method);
}

void LogDiagnostics(const Operation* op, String message, String method)
{
    LogDiagnostics(*op, message, method);
}

void LogDiagnostics(const Token& token, String message, String method)
{
    DebugDumpObjectToLog(DisplayString(token), message, method);
}

void LogDiagnostics(const Token* token, String message, String method)
{
    LogDiagnostics(*token, message, method);
}

void LogDiagnostics(const TokenList& tokenList, String message, String method)
{
    DebugDumpObjectToLog(DisplayString(tokenList), message, method);
}

void LogDiagnostics(const TokenList* tokenList, String message, String method)
{
    LogDiagnostics(*tokenList, message, method);
}

void LogDiagnostics(const ObjectReferenceMap& map, String message, String method)
{
    DebugDumpObjectToLog(DisplayString(map), message, method);
}

#endif
