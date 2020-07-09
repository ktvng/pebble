#ifndef __PARSE_H
#define __PARSE_H

#include <string>
#include <vector>
#include <fstream>
#include <list>
#include <iostream>

#include "abstract.h"



// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// a PrecedenceClass is a collection of String representing operation symbols that all have the same precedence
/// when parsed
class PrecedenceClass
{
    public:
    std::vector<String> Members;
    bool Contains(String symb);
};

/// a struct containg information about each atomic production rule in the language's formal CFG
/// [Name] is the rule name, which also determines [OpType]
/// [Symbol] is the symbol representing the operation/precedence of the rule
/// [Precedence] is the precedence of the rule, derived from [Symbol]
/// [OpType] is the atomic operation which the rule corresponds to
/// [IntoPattern] is the pattern which applying the rule yields
/// [FromProduction] is the production rule name
/// [ParseMethod] is the method which describes how to reverse the rule on a set of operand Operations
struct CFGRule
{
    String Name;
    String Symbol;
    int Precedence;
    OperationType OpType;
    std::vector<String> IntoPattern;
    String FromProduction;
    String ParseMethod;

    bool HasHigherPrecedenceClassOverride;
    bool HasLowerPrecedenceClassOverride;
    PrecedenceClass HigherPrecedenceClass;
    PrecedenceClass LowerPrecedenceClass;
};

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


// ---------------------------------------------------------------------------------------------------------------------
// Static definitions

/// the [Grammar] consists of a list of production rules represented as [CFGRules]
static std::vector<CFGRule*> Grammar;

/// the [PrecedenceRules] are a linked list of PrecedenceClasses order from lowest precedence to highest precedence
static std::list<PrecedenceClass> PrecedenceRules;

/// the production variables are the string representations of the possible production rules
static std::vector<String> ProductionVariables = { "Ref" };



// ---------------------------------------------------------------------------------------------------------------------
// Methods

/// parses a [line] of code into an AST of Operations
Operation* ExpressionParser(TokenList& line);

/// compiles the [Grammar] from ./assets/grammar.txt
void CompileGrammar();

#endif
