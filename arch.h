#ifndef __ARCH_H
#define __ARCH_H

#include <vector>
#include <string>

#include "main.h"
#include "utils.h"

// Structs
struct Reference
{
    std::string Name;
    Object* ToObject;
};

struct Object
{
    ObjectClass Class;
    std::vector<Reference*> Attributes;
    std::vector<Method*> Methods;
    void* Value;
};

struct OperationTypeProbability
{
    OperationType Type;
    double Probability;
};

enum LineType
{
    IfLine,
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
    int Level;
};

struct Program
{
    std::vector<CodeLine> Lines;
    Scope* GlobalScope;
    std::vector<Block*> Blocks;
    std::vector<ObjectReferenceMap*> ObjectsIndex;
};

struct Method
{
    String* Name;
    Block* CodeBlock;
    Scope* Parameters;
};

#endif
