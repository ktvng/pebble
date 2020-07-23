#ifndef __GRAMMAR_H
#define __GRAMMAR_H

#include <list>

#include "abstract.h"

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

struct PreprocessorRule
{
    String Becomes;
    std::vector<String> Pattern;
};


// ---------------------------------------------------------------------------------------------------------------------
// Definitions

/// the [Grammar] consists of a list of production rules represented as [CFGRules]
extern std::vector<CFGRule> Grammar;

/// the [PrecedenceRules] are a linked list of PrecedenceClasses order from lowest precedence to highest precedence
extern std::list<PrecedenceClass> PrecedenceRules;

/// the production variables are the string representations of the possible production rules
extern std::vector<String> ProductionVariables;

extern std::vector<PreprocessorRule> PreprocessorRules;

/// compiles the [Grammar] from ./assets/grammar.txt
void CompileGrammar();

int PrecedenceOf(String opSymbol);
int PrecedenceOf(Token* lookaheadToken);

inline const String g_ContextStartSymbol = "@";

#endif
