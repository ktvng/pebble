#ifndef __MAIN_H
#define __MAIN_H

#include <string>
#include <vector>
#include <map>



struct Reference;
struct Object;
struct Block;
struct Operation;
struct Token;
struct OperationTypeProbability;
struct SystemMessage;

typedef std::vector<Token*> TokenList;
typedef std::string ObjectClass;
typedef std::string String;
typedef std::vector<OperationTypeProbability> PossibleOperationsList;
typedef std::vector<Operation*> OperationsList;


const ObjectClass IntegerClass = "Integer";
const ObjectClass DecimalClass = "Decimal";
const ObjectClass StringClass = "String";
const ObjectClass BooleanClass = "Boolean";
const ObjectClass NullClass = "Null";

const std::string c_returnReferenceName = "ReturnedObject";

/// defines how severe a log event is. due to enum -> int casting, definition order is important
enum LogSeverityType
{
    Sev1_Notify,
    Sev2_Important,
    Sev3_Critical,
};

const std::map<LogSeverityType, String> LogSeverityTypeString =
{
    { LogSeverityType::Sev3_Critical, "Sev3" },
    { LogSeverityType::Sev2_Important, "Sev2" },
    { LogSeverityType::Sev1_Notify, "Sev1" },
};
/// log all messages at or above this level
const LogSeverityType LogSeverityLevel = LogSeverityType::Sev1_Notify;


const bool c_DEBUG = true;
const bool c_ERROR = true;

extern std::vector<SystemMessage> RuntimeMsgBuffer;
extern std::vector<SystemMessage> CompileMsgBuffer;

Operation* ParseLine(TokenList& tokens);
TokenList LexLine(const std::string& line);
Reference* DecideReferenceOf(Token* token);

#endif