#ifndef __ARCH_H
#define __ARCH_H

#include <vector>
#include <string>

#include "main.h"
#include "utils.h"

// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// functionally a named pointer, Chief uses these as an interface to refer to objects. Objects are never
/// called directly
/// [Name] is the name which is used to call the Reference
/// [To] a Referable (Method/Object) 
struct Reference
{
    std::string Name;
    Referable* To = nullptr;
};

/// holds information on the probability that a given line is some operation with
/// [Type] as the OperationType
/// [Probability] the liklihood of this [Type]
struct OperationTypeProbability
{
    OperationType Type;
    double Probability;
};

/// possibly deprecated
enum LineType
{
    IfLine,
    WhileLine,
    Composite,
    Atomic,
};

/// used in tokenization to classify string bits
enum class TokenType
{
    Simple,                 // any general word/symbol not matching another type
    Reference,              // encodes a Reference name; must start with capital letter
    Integer,                // a whole number
    String,                 // a line of characters enclosed by double quotes
    Decimal,                // a number with a decimal point
    Boolean,                // true of false in any case
};

/// used in tokenization, it contains metainformation about a string bit such as
/// [Type] describing the token's TokenType
/// [Content] being the string bit it refers to
/// [Position] what position in a line does the token occur
struct Token
{
    TokenType Type;
    String Content;
    int Position;
};

/// a list of TokenTypes which represent primitive objects
const std::vector<TokenType> PrimitiveTokenTypes {
    TokenType::Integer,
    TokenType::String,
    TokenType::Decimal,
    TokenType::Boolean,
};

/// tokens which represent any Object 
const std::vector<TokenType> ObjectTokenTypes {
    TokenType::Integer,
    TokenType::String,
    TokenType::Decimal,
    TokenType::Boolean,
    TokenType::Reference,
};

/// used to keep track of objects and their references
struct ObjectReferenceMap
{
    Object* Object;
    std::vector<Reference*> References;
};

/// a scope is the context in which the compiler and runtime environment resolve references
/// and add new references.
/// [ReferencesIndex] contains all references available in the scope
/// [InheritedScope] is a link to the parent scope and to inherited references
struct Scope
{
    std::vector<Reference*> ReferencesIndex;
    Scope* InheritedScope;
};

/// represents the result of tokenizing a line with additional metadata
/// [Tokens] are the tokens resulting from processing an effective line of code
/// [LineNumber] is the line of code which was processed
/// [Level] is the indent level of the line
struct CodeLine
{
    TokenList Tokens;
    int LineNumber;
    int Level;
};

/// a structure which contains a runnable representation of Chief code
/// [Lines] are the 'raw' tokenized CodeLines of the program
/// [GlobalScope] (possible deprecated) is the global scope of the program
/// [ObjectsIndex] contains all objects and their references during program execution
struct Program
{
    std::vector<CodeLine> Lines;
    Scope* GlobalScope;
    std::vector<Block*> Blocks;
    std::vector<ObjectReferenceMap*> ObjectsIndex;
};

#endif
