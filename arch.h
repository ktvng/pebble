#ifndef __ARCH_H
#define __ARCH_H

#include <vector>
#include <string>

#include "main.h"

// Structs
enum class ControlType
{
    If,
    While,
    Statement
};

enum class OperationType
{
    Define,
    Assign, //
    IsEqual,
    LessThan,
    GreaterThan,
    Add, //
    Subtract,
    Multiply,
    Divide,
    And, //
    Or,
    Not,
    Evaluate,
    Print,
    Return,
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

struct LineTypeProbability
{
    OperationType Type;
    double Probability;
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
};

#endif
