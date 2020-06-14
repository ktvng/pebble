#ifndef __ARCH_H
#define __ARCH_H

#include <vector>
#include <string>

#include "main.h"

// Structs
enum class OperationType
{
    Define, 
    Assign, 
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
    Return, //
};

struct Reference
{
    std::string Name;
    Object* ToObject;
};

struct Object
{
    ObjectClass Class;
    std::vector<Reference> Attributes;
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

#endif
