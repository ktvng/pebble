#include <fstream>

#include "grammar.h"

#include "token.h"
#include "diagnostics.h"


std::vector<CFGRule> Grammar;
std::list<PrecedenceClass> PrecedenceRules;
std::vector<String> ProductionVariables;
std::vector<PreprocessorRule> PreprocessorRules;

// ---------------------------------------------------------------------------------------------------------------------
// PrecedenceClass
bool PrecedenceClass::Contains(String symb)
{
    for(auto str: Members)
        if(str == symb)
            return true;

    return false;
}



// ---------------------------------------------------------------------------------------------------------------------
// Formal grammar parser grammar parser
// aka the formal parser for the grammar of the formal grammar parser

OperationType StringNameToOperationType(String Name)
{
    if(Name=="Add")
        return OperationType::Add;
    else if(Name=="Subtract")
        return OperationType::Subtract;
    else if(Name=="Multiply")
        return OperationType::Multiply;
    else if(Name=="Divide")
        return OperationType::Divide;

    else if(Name=="And")
        return OperationType::And;
    else if(Name=="Or")
        return OperationType::Or;
    else if(Name=="Not")
        return OperationType::Not;
        
    else if(Name=="Is")
        return OperationType::Is;
    else if(Name=="IsEqual")
        return OperationType::IsEqual;
    else if(Name=="IsNotEqual")
        return OperationType::IsNotEqual;
    else if(Name=="IsGreaterThan")
        return OperationType::IsGreaterThan;
    else if(Name=="IsLessThan")
        return OperationType::IsLessThan;
    else if(Name=="IsGreaterThanOrEqualTo")
        return OperationType::IsGreaterThanOrEqualTo;
    else if(Name=="IsLessThanOrEqualTo")
        return OperationType::IsLessThanOrEqualTo;

    else if(Name=="Evaluate")
        return OperationType::Evaluate;
    else if(Name=="EvaluateHere")
        return OperationType::EvaluateHere;

    else if(Name=="If")
        return OperationType::If;
    else if(Name=="ElseIf")
        return OperationType::ElseIf;
    else if(Name=="Else")
        return OperationType::Else;
    else if(Name=="While")
        return OperationType::While;
    else if(Name=="DefineMethod")
        return OperationType::DefineMethod;
    else if(Name=="Assign")
        return OperationType::Assign;
    else if(Name=="Tuple")
        return OperationType::Tuple;
    else if(Name=="Print")
        return OperationType::Print;
    else if(Name=="Ask")
        return OperationType::Ask;
    else if(Name=="Return")
        return OperationType::Return;

    else if(Name=="New")
        return OperationType::New;
    else if(Name=="ScopeResolution")
        return OperationType::ScopeResolution;
    else if(Name=="Class")
        return OperationType::Class;
    else if(Name=="Ref")
        return OperationType::Ref;
    else if(Name=="NoOperationType")
        return OperationType::NoOperationType;
    
    LogIt(LogSeverityType::Sev2_Important, "StringNameToOperationType", Msg("possibly unimplemented enum type %s", Name));
    return OperationType::NoOperationType;
}


void AddProductionVariable(String newProductionVar)
{
    for(auto productionVar: ProductionVariables)
    {
        if(productionVar == newProductionVar)
            return;
    }
    ProductionVariables.push_back(newProductionVar);
}

int PrecedenceOf(String opSymbol)
{
    int i=1;
    for(auto rule: PrecedenceRules)
    {
        for(auto str: rule.Members)
        {
            if(str == opSymbol)
            {
                return i;
            }
        }
        i++;
    }
    LogIt(Sev2_Important, "PrecedenceOf", Msg("unknown operation symbol %s", opSymbol));
    return PrecedenceRules.size() - 1;
}

int PrecedenceOf(Token* lookaheadToken)
{
    if(lookaheadToken == nullptr)
        return 0;

    return PrecedenceOf(lookaheadToken->Content);
}

void AssignRulePrecedences()
{
    for(auto& rule: Grammar)
    {
        rule.Precedence = PrecedenceOf(rule.Symbol);
    }
}

void AddPrecedenceClass(TokenList& tokens)
{
    PrecedenceClass precdence;
    for(Token* t: tokens)
    {
        precdence.Members.push_back(t->Content);
    }
    PrecedenceRules.push_front(precdence);
}

void AddGrammarRuleInternal(
    TokenList&tokens, 
    String& name, 
    String& symbol, 
    String& parseMethod, 
    PrecedenceClass& higherpclass, 
    PrecedenceClass& lowerpclass)
{
    CFGRule rule;

    rule.HasHigherPrecedenceClassOverride = false;
    rule.HasLowerPrecedenceClassOverride = false;

    rule.Name = name;
    rule.Symbol = symbol; 
    rule.ParseMethod = parseMethod; 

    rule.OpType = StringNameToOperationType(rule.Name);
    rule.FromProduction = tokens.at(0)->Content;
    AddProductionVariable(rule.FromProduction);

    if(!higherpclass.Members.empty())
    {
        rule.HasHigherPrecedenceClassOverride = true;
        rule.HigherPrecedenceClass = higherpclass;
    }

    if(!lowerpclass.Members.empty())
    {
        rule.HasLowerPrecedenceClassOverride = true;
        rule.LowerPrecedenceClass = lowerpclass;
    }

    for(size_t i=3; i<tokens.size(); i++)
    {
        rule.IntoPattern.push_back(tokens.at(i)->Content);
    }

    Grammar.push_back(rule);
}

PrecedenceClass GetOverridePrecedenceClass(TokenList& tokens)
{
    PrecedenceClass pclass;
    pclass.Members.reserve(tokens.size()-1);
    for(size_t i=1; i<tokens.size(); i++)
    {
        pclass.Members.push_back(tokens.at(i)->Content);
    }

    return pclass;
}

void AddGrammarRule(TokenList& tokens)
{
    static String name;
    static String symbol;
    static String parseMethod;
    static PrecedenceClass higherpclass;
    static PrecedenceClass lowerpclass;

    if(tokens.at(0)->Content == g_ContextStartSymbol)
    {
        name = tokens.at(1)->Content;
        symbol = tokens.at(2)->Content;
        parseMethod = tokens.at(3)->Content;
        higherpclass.Members.clear();
        lowerpclass.Members.clear();
        
    }
    else if(tokens.at(0)->Content == ">")
    {
        if(name.empty())
        {
            LogIt(LogSeverityType::Sev3_Critical, "AddGrammarRule", "expected context declaration before precedence override");
            return;
        }

        higherpclass = GetOverridePrecedenceClass(tokens);
    }
    else if(tokens.at(0)->Content == "<")
    {
        if(name.empty())
        {
            LogIt(LogSeverityType::Sev3_Critical, "AddGrammarRule", "expected context declaration before precedence override");
            return;
        }

        lowerpclass = GetOverridePrecedenceClass(tokens);
    }
    else if(tokens.at(0)->Type == TokenType::Reference)
    {
        if(name.empty())
        {
            LogIt(LogSeverityType::Sev3_Critical, "AddGrammarRule", "expected context declaration before pattern");
            return;
        }

        AddGrammarRuleInternal(tokens, name, symbol, parseMethod, higherpclass, lowerpclass);
    }
}

void AddPreprocessorRule(TokenList& tokens)
{
    static String becomes;

    if(tokens[0]->Content == g_ContextStartSymbol)
    {
        if(tokens.size() != 2)
        {
            LogIt(LogSeverityType::Sev3_Critical, "AddPreprocessorRule", "context declaration (@) line has wrong # of args");
            return;
        }

        becomes = tokens[1]->Content;
    }
    else
    {
        if(becomes.empty())
        {
            LogIt(LogSeverityType::Sev3_Critical, "AddPreprocessorRule", "pattern reached before context declaration (@)");
            return;
        }

        std::vector<String> pattern;
        for(auto token: tokens)
        {
            pattern.push_back(token->Content);
        }
        PreprocessorRule rule = { becomes, pattern };
        PreprocessorRules.push_back(rule);
    }
}

void ResetGrammar()
{
    Grammar.clear();
    Grammar.reserve(128);

    PrecedenceRules.clear();
    
    ProductionVariables.clear();
    ProductionVariables.reserve(32);
    ProductionVariables.push_back("Ref");

    PreprocessorRules.clear();
    PreprocessorRules.reserve(16);
}

void CompileGrammar()
{
    ResetGrammar();

    std::fstream file;
    file.open("./assets/grammar.txt", std::ios::in);

    // state: +1 upon every line with first non-whitespace char being ';'
    //  0: skip all lines
    //  1: read preprocessor rules
    //  2: read grammar rules 
    //  3: read precedences
    //  4+: skip all lines
    int state = 0;

    String line;
    while(std::getline(file, line))
    {
        /// TODO: we should't have to lex the line every time
        TokenList tokens = LexLine(line);
        if(tokens.empty())
        {
            continue;
        }

        if(tokens.at(0)->Content == ";")
        {
            DeleteTokenList(tokens);
            state++;
            continue;
        }

        switch(state)
        {
            case 1:
            AddPreprocessorRule(tokens);
            break;

            case 2:
            AddGrammarRule(tokens);
            break;;

            case 3:
            AddPrecedenceClass(tokens);
            break;

            default:
            break;
        }

        DeleteTokenList(tokens);
    }

    AssignRulePrecedences();
}
