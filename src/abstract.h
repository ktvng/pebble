#ifndef __ABSTRACT_H
#define __ABSTRACT_H

#include <string>
#include <vector>
#include <map>

union Value;

struct Token;
struct OperationTypeProbability;
struct SystemMessage;
struct ObjectReferenceMap;
struct Scope;
struct CodeLine;
struct Program;
struct Reference;
struct ByteCodeInstruction;
struct CFGRule;
struct Call;

class Object;
class Method;

class Executable;
class Block;
class Operation;

class PrecedenceClass;

typedef std::string String;
typedef std::string ObjectClass;
typedef std::vector<Token*> TokenList;
typedef std::vector<Operation*> OperationsList;
typedef std::vector<std::string> ParameterList;

typedef unsigned long long extArg_t;


/// log events are used for internal Pebble developer debugging
/// defines how severe a log event is. due to enum -> int casting, definition order is important
enum LogSeverityType
{
    Sev0_Debug,
    Sev1_Notify,
    Sev2_Important,
    Sev3_Critical,
};

/// different atomic operations which build Pebble code
enum OperationType
{
    Assign,                         // change what a Reference points to

    Is,
    IsEqual,                        // returns Reference to BooleanClass result of ==
    IsNotEqual,                     // returns Reference to BooleanClass result of !=

    IsLessThan,                     // returns Reference to BooleanClass result of <
    IsGreaterThan,                  // returns Reference to BooleanClass result of >
    IsLessThanOrEqualTo,            // returns Reference to BooleanClass result of <=
    IsGreaterThanOrEqualTo,         // returns Reference to BooleanClass result of >=

    Add,                            // returns Reference to result of +
    Subtract,                       // returns Reference to result of -
    Multiply,                       // returns Reference to result of *
    Divide,                         // returns Reference to result of /

    And,                            // returns Reference to result of &&
    Or,                             // returns Reference to result of ||
    Not,                            // returns Reference to result of !

    Evaluate,                       // returns Reference to result of method call
    EvaluateHere,

    Print,                          // prints a Ref to the screen
    Ask,
    
    Ref,                            // terminal of an operation tree, returns a Reference
    DefineMethod,                   // add a new method Reference to scope, but returns NullReference
    Return,                         // break out of a method and return a value

    If,                             // conditionally executes the next block of code
    ElseIf,
    Else,
    While,
    
    Tuple,                          // constructs and returns a (>1) ordering of references
    
    New,
    ScopeResolution,
    DoTypeBinding,

    Array,

    NoOperationType,
};

enum class SystemMessageType
{
    Exception,
    Warning,
    Advice
};

#endif