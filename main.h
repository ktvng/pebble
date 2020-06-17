#ifndef __MAIN_H
#define __MAIN_H

#include <string>
#include <vector>
#include <map>

#include "utils.h"

enum class ExecutableType
{
    Operation,
    Block
};

class Executable 
{
    public:
    ExecutableType ExecType;
};


struct Reference;
struct Object;
struct Token;
struct OperationTypeProbability;
struct SystemMessage;
struct ObjectReferenceMap;
struct Scope;
struct CodeLine;
struct Program;


enum OperationType
{
    Define, // defines a new reference in the scope special
    Assign, // special
    IsEqual, //
    IsLessThan, //
    IsGreaterThan, //
    Add, 
    Subtract, //
    Multiply, //
    Divide, //
    And, 
    Or, //
    Not, //
    Evaluate, //
    Print, //
    Return, // special

    If,
    EndLabel,
};

class Operation : public Executable 
{
    public:
    OperationType Type;
    std::vector<Operation*> Operands;
    Reference* Value;
    int LineNumber;
};
class Block : public Executable
{
    public:
    Scope* LocalScope;
    std::vector<Executable*> Executables;
};


typedef std::vector<Token*> TokenList;
typedef std::string ObjectClass;
typedef std::string String;
typedef std::vector<OperationTypeProbability> PossibleOperationsList;
typedef std::vector<Operation*> OperationsList;


inline const ObjectClass IntegerClass = "Integer";
inline const ObjectClass DecimalClass = "Decimal";
inline const ObjectClass StringClass = "String";
inline const ObjectClass BooleanClass = "Boolean";
inline const ObjectClass NullClass = "Null";

inline const std::string c_returnReferenceName = "ReturnedObject";
inline const std::string c_primitiveObjectName = "PrimitiveObject";
inline const std::string c_operationReferenceName = "OperationReference";

/// defines how severe a log event is. due to enum -> int casting, definition order is important
enum LogSeverityType
{
    Sev0_Debug,
    Sev1_Notify,
    Sev2_Important,
    Sev3_Critical,
};

const std::map<LogSeverityType, String> LogSeverityTypeString =
{
    { LogSeverityType::Sev3_Critical, "Critical" },
    { LogSeverityType::Sev2_Important, "Important" },
    { LogSeverityType::Sev1_Notify, "Notify" },
    { LogSeverityType::Sev0_Debug, "Debug" }
};


Operation* ParseLine(Scope* scope, TokenList& tokens);
TokenList LexLine(const std::string& line);
Reference* DoOperation(Scope* scope, Operation* op);

void ReassignReference(Reference* ref, Object* newObj);


Reference* DefineNewReference(String refName, int value, Scope* scope);
Reference* DefineNewReference(String refName, bool value, Scope* scope);
Reference* DefineNewReference(String refName, double value, Scope* scope);
Reference* DefineNewReference(String refName, String value, Scope* scope);

Reference* DefineNewReference(String refName, Object* obj, Scope* scope);
Reference* DefineNewReference(String refName, Token* valueToken, Scope* scope);

Reference* DefineNullReference(String refName, Scope* scope);
Reference* DefineNullReference(Scope* scope);

void AddReferenceToScope(Reference* ref, Scope* scope);


Reference* ReferenceFor(Token* token, Scope* scope, String refName = c_returnReferenceName);

#endif