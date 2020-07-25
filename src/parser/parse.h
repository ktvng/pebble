#ifndef __PARSE_H
#define __PARSE_H

#include <string>
#include <vector>

#include "abstract.h"

// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// a parsetoken is the evoled from of a token from the tokenizer (token.cpp) which contains additional meta
/// information and a fragment of the AST of Operations
/// [TokenType] is a string describing the the token in terms of its ProductionVariable
/// [Value] is the fragment of the operation tree 
/// [Next] is the next ParseToken in the partially parsed line
/// [Prev] is the previous ParseToken in the partially parsed line
struct ParseToken
{
    String TokenType;
    Operation* Value;
    ParseToken* Next = nullptr;
    ParseToken* Prev = nullptr;
}; 

extern std::vector<Object*> ConstantPrimitiveObjects;

// ---------------------------------------------------------------------------------------------------------------------
// Methods

/// parses a [line] of code into an AST of Operations
Operation* ExpressionParser(TokenList& line);

#endif
