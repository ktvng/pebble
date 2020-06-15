#ifndef __ARCH_H
#define __ARCH_H

#include <vector>
#include <string>

#include "main.h"
#include "utils.h"

// Structs
enum class OperationType
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
};

struct Reference
{
    std::string Name;
    Object* ToObject;
};

struct Object
{
    ObjectClass Class;
    std::vector<Reference*> Attributes;
    void* Value;
};

struct Operation
{
    OperationType Type;
    std::vector<Operation*> Operands;
    Reference* Value;
    int LineNumber;
};

struct Block
{
    Scope* LocalScope;
    std::vector<Operation*> Operations;
};

struct OperationTypeProbability
{
    OperationType Type;
    double Probability;
};

enum LineType
{
    If,
    While,
    Composite,
    Atomic,
};

enum class TokenType
{
    Simple,
    Reference,
    Integer,
    String,
    Decimal,
    Boolean,
};

struct Token
{
    TokenType Type;
    String Content;
    int Position;
};

const std::vector<TokenType> PrimitiveTokenTypes {
    TokenType::Integer,
    TokenType::String,
    TokenType::Decimal,
    TokenType::Boolean,
};

const std::vector<TokenType> ObjectTokenTypes {
    TokenType::Integer,
    TokenType::String,
    TokenType::Decimal,
    TokenType::Boolean,
    TokenType::Reference,
};

struct ObjectReferenceMap
{
    Object* Object;
    std::vector<Reference*> References;
};

struct Scope
{
    std::vector<Reference*> ReferencesIndex;
    Scope* InheritedScope;
};

struct CodeLine
{
    TokenList Tokens;
    int LineNumber;
};

struct Program
{
    std::vector<CodeLine> Lines;
    Scope* GlobalScope;
    std::vector<Block> Blocks;
    std::vector<ObjectReferenceMap*> ObjectsIndex;
};

#endif
