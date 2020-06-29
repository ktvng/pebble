#ifndef __MAIN_H
#define __MAIN_H

#include <string>
#include <vector>
#include <map>

#include "utils.h"

// ---------------------------------------------------------------------------------------------------------------------
// Declarations

struct Token;
struct OperationTypeProbability;
struct SystemMessage;
struct ObjectReferenceMap;
struct Scope;
struct CodeLine;
struct Program;
struct Method;
struct Reference;
struct Block;


// ---------------------------------------------------------------------------------------------------------------------
// Referables (Objects/Methods)

/// defines the class of an object which imbues type properties
typedef std::string ObjectClass;

/// type of a referable object
enum class ReferableType
{
    Object,
    Method
};

/// abstract class for anything that a Reference can point to
class Referable
{
    public:
    ReferableType Type;
    
};

/// emulated object in Chief
/// [Class] refers to the Object Class which governs type properties
/// [Attributes] are references to other Objects/Methods
/// [Value] is used by primitive objects for basic operations.
///         there are currently only int*, double*, bool*, std::string* value types
class Object : public Referable
{
    public:
    ObjectClass Class;
    std::vector<Reference*> Attributes;
    void* Value;
};

/// emulated method in Chief
/// [CodeBlock] is the code associated with the method
/// [Parameters] are the parameters input to the method
class Method : public Referable
{
    public:
    Block* CodeBlock;
    Scope* Parameters;
};


// ---------------------------------------------------------------------------------------------------------------------
// Referables (Objects/Methods)

/// different atomic operations which build Chief code
enum OperationType
{
    Define,                         // add a new Reference to scope
    Assign,                         // change what a Reference points to
    IsEqual,                        // returns Reference to BooleanClass result of ==
    IsLessThan,                     // returns Reference to BooleanClass result of <
    IsGreaterThan,                  // returns Reference to BooleanClass result of >

    Add,                            // returns Reference to result of +
    Subtract,                       // returns Reference to result of -
    Multiply,                       // returns Reference to result of *
    Divide,                         // returns Reference to result of /

    And,                            // returns Reference to result of &&
    Or,                             // returns Reference to result of ||
    Not,                            // returns Reference to result of !
    Evaluate,                       // returns Reference to result of method call
    Print,                          // prints a Ref to the screen
    
    Ref,                            // terminal of an operation tree, returns a Reference
    DefineMethod,                   // add a new method Reference to scope, but returns NullReference
    Return,                         // break out of a method and return a value

    If,                             // conditionally executes the next block of code
    EndLabel,                       // end of an if statement
    Tuple,                          // constructs and returns a (>1) ordering of references
};

/// type of executable, either a single line (Operation), or list of lines (Block)
enum class ExecutableType
{
    Operation,
    Block
};

/// abstract class for anything build from operations
class Executable 
{
    public:
    ExecutableType ExecType;
};

/// a representation of an atomic operation with
/// [Type] which governs how the operation should be executed
/// [Operands] which will be evaluated into the inputs to this operation
/// [Value] which is only used for OperationType::Ref (terminals) of the operation tree
/// [LineNumber] which is the line of code that the operation was parsed from
class Operation : public Executable 
{
    public:
    OperationType Type;
    std::vector<Operation*> Operands;
    Reference* Value;
    int LineNumber;
};

/// a sequential list of atomic operations/Blocks is a block
/// [Executables] is the list child Execuable objects
class Block : public Executable
{
    public:
    std::vector<Executable*> Executables;
};


// ---------------------------------------------------------------------------------------------------------------------
// Typedefs

typedef std::vector<Token*> TokenList;
typedef std::string String;
typedef std::vector<OperationTypeProbability> PossibleOperationsList;
typedef std::vector<Operation*> OperationsList;


// ---------------------------------------------------------------------------------------------------------------------
// Constants

inline const ObjectClass IntegerClass = "Integer";
inline const ObjectClass DecimalClass = "Decimal";
inline const ObjectClass StringClass = "String";
inline const ObjectClass BooleanClass = "Boolean";
inline const ObjectClass NullClass = "Nothing";
inline const ObjectClass ArrayClass = "Array";
inline const ObjectClass TupleClass = "Tuple";

/// used when an operation returns a temporary reference. will only have lifespan of 1-2 lines of code, often less
/// as it will be dereferenced if it is an operand to a parent operation
inline const std::string c_temporaryReferenceName = "!TemporaryReference";

/// used when returning values by OperationType::Return
inline const std::string c_returnReferenceName = "!ReturnedReference";

/// used for primitive objects resolved during compile time and incorporated as constants in an operation tree
inline const std::string c_operationReferenceName = "!ConstPrimitive";


// ---------------------------------------------------------------------------------------------------------------------
// Logging and debugging

/// log events are used for internal Chief developer debugging
/// defines how severe a log event is. due to enum -> int casting, definition order is important
enum LogSeverityType
{
    Sev0_Debug,
    Sev1_Notify,
    Sev2_Important,
    Sev3_Critical,
};

/// converts LogSeverityType to a printable string
const std::map<LogSeverityType, String> LogSeverityTypeString =
{
    { LogSeverityType::Sev3_Critical, "Critical" },
    { LogSeverityType::Sev2_Important, "Important" },
    { LogSeverityType::Sev1_Notify, "Notify" },
    { LogSeverityType::Sev0_Debug, "Debug" }
};

/// if true will turn on output for 'print' command of chief
extern bool g_outputOn;


#endif
