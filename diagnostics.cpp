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
#include <algorithm>



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

String IndentStringToLevel(String str, int level, int margin=0)
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

String StringForAttrbute(String name, String value)
{
    return MSG("-[%s]: %s\n", name, value);
}

String StringForAttrbuteOneLine(String name, String value)
{
    return MSG("-[%s]: %s", name, value);
}

int ValueStartOffset(String name)
{
    return 5 + name.size();
}


template <typename T>
String ToString(std::vector<T> list, String typeString)
{
    if(list.size() == 0)
        return MSG("<List<%s>", typeString);
    String listString = MSG("<List<%s>\n", typeString);
    for(size_t i=0; i<list.size(); i++)
    {
        int offset = ValueStartOffset(std::to_string(i));
        listString += IndentLevel(1) +
             StringForAttrbuteOneLine(std::to_string(i), IndentStringToLevel(ToString(list.at(i)), 1, offset));
    }
    return listString;
}

String ToString(const String& str)
{
    return str;
}


// Diagnostic printing
String ToString(const Object& obj)
{
    String objString = "<Object>\n";
    
    objString += IndentLevel(1) + 
        StringForAttrbute("Class", obj.Class);

    objString += IndentLevel(1) +
        StringForAttrbute("Value", GetStringValue(obj));
    
    objString += IndentLevel(1) +
        StringForAttrbute("Attributes", IndentStringToLevel(ToString(obj.Attributes, "Reference"), 1));

    return objString;
}

String ToString(const Object* obj)
{
    return ToString(*obj);
}

String ToString(const Reference& ref)
{
    String refString = "<Reference>\n";

    refString += IndentLevel(1) +
        StringForAttrbute("Name", ref.Name);

    refString += IndentLevel(1) +
        StringForAttrbute("ToObject", IndentStringToLevel(ToString(ref.ToObject), 1));

    return refString;
}

String ToString(const Reference* ref)
{
    return ToString(*ref);
}

String ToString(const Operation& op){
    return "OP";
}

String ToString(const ObjectReferenceMap& map)
{
    String mapString = "<ObjectReferenceMap>\n";
    mapString += IndentLevel(1) + 
        StringForAttrbute("Object", IndentStringToLevel(ToString(*map.Object),1));

    mapString += IndentLevel(1) + StringForAttrbute("Reference", "<List<Reference*>");
    for(size_t i=0; i<map.References.size(); i++)
    {
        mapString += IndentLevel(2) + 
            StringForAttrbute(std::to_string(i), map.References.at(i)->Name);
    }

    return mapString;
}

String ToString(const ObjectReferenceMap* map)
{
    return ToString(*map);
}

String ToString(const Token& token)
{
    return StringForAttrbute("Token", MSG("Type: %s\t Content: %s", 
        GetStringTokenType(token.Type),
        token.Content));
}

String ToString(const Token* token)
{
    return ToString(*token);
}

void PrintDiagnostics(const Object& obj)
{
    std::cout << ToString(obj);
}

void PrintDiagnostics(const Object* obj)
{
    std::cout << ToString(obj);
}

void PrintDiagnostics(const Reference& ref)
{
    std::cout << ToString(ref);
}

void PrintDiagnostics(const Reference* ref)
{
    std::cout << ToString(ref);
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
    std::cout << ToString(token);
}

void PrintDiagnostics(const Token* token)
{
    PrintDiagnostics(*token);
}

void PrintDiagnostics(const TokenList& tokenList)
{
    std::cout << ToString(tokenList, "Token*");
}

void PrintDiagnostics(const TokenList* tokenList)
{
    PrintDiagnostics(*tokenList);
}

void PrintDiagnostics(const ObjectReferenceMap& map)
{
    std::cout << ToString(map);
}

void PrintDiagnostics(const ObjectReferenceMap* map)
{
    std::cout << ToString(*map);
}
