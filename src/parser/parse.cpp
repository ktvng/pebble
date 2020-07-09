#include "parse.h"

#include "main.h"
#include "token.h"
#include "diagnostics.h"
#include "reference.h"
#include "operation.h"
#include "scope.h"
#include "object.h"
#include "program.h"

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
// Formal grammar parser parser

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
    else if(Name=="If")
        return OperationType::If;
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
    else if(Name=="Return")
        return OperationType::Return;

    else if(Name=="New")
        return OperationType::New;
    else if(Name=="ScopeResolution")
        return OperationType::ScopeResolution;
    else if(Name=="Class")
        return OperationType::Class;

    else
        return OperationType::Ref;
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

// TODO: move to diagnostics
void PrintPrecedenceRules()
{
    std::cout << "PRINTING\n";
    for(auto rule: PrecedenceRules)
    {
        for(auto s: rule.Members)
        {
            std::cout << s << " ";
        }
        std::cout << "\n";
    }
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
    LogIt(Sev3_Critical, "PrecedenceOf", Msg("unknown operation symbol %s", opSymbol));
    return 0;
}

int PrecedenceOf(Token* lookaheadToken)
{
    if(lookaheadToken == nullptr)
        return 0;

    return PrecedenceOf(lookaheadToken->Content);
}

void AssignRulePrecedences()
{
    for(auto rule: Grammar)
    {
        rule->Precedence = PrecedenceOf(rule->Symbol);
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
    CFGRule* rule = new CFGRule;

    rule->HasHigherPrecedenceClassOverride = false;
    rule->HasLowerPrecedenceClassOverride = false;

    rule->Name = name;
    rule->Symbol = symbol; 
    rule->ParseMethod = parseMethod; 

    rule->OpType = StringNameToOperationType(rule->Name);
    rule->FromProduction = tokens.at(0)->Content;
    AddProductionVariable(rule->FromProduction);

    if(!higherpclass.Members.empty())
    {
        rule->HasHigherPrecedenceClassOverride = true;
        rule->HigherPrecedenceClass = higherpclass;
    }

    if(!lowerpclass.Members.empty())
    {
        rule->HasLowerPrecedenceClassOverride = true;
        rule->LowerPrecedenceClass = lowerpclass;
    }

    for(size_t i=3; i<tokens.size(); i++)
    {
        rule->IntoPattern.push_back(tokens.at(i)->Content);
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

    if(tokens.at(0)->Content == "@")
    {
        name = tokens.at(1)->Content;
        symbol = tokens.at(2)->Content;
        parseMethod = tokens.at(3)->Content;
        higherpclass.Members.clear();
        lowerpclass.Members.clear();
        
    }
    else if(tokens.at(0)->Content == ">")
    {
        higherpclass = GetOverridePrecedenceClass(tokens);
    }
    else if(tokens.at(0)->Content == "<")
    {
        lowerpclass = GetOverridePrecedenceClass(tokens);
    }
    else if(tokens.at(0)->Type == TokenType::Reference)
    {
        AddGrammarRuleInternal(tokens, name, symbol, parseMethod, higherpclass, lowerpclass);
    }
}

void CompileGrammar()
{
    std::fstream file;
    file.open("./assets/grammar.txt", std::ios::in);


    // state: +1 upon every occurance of '###'
    //  0: skip all lines
    //  1: read grammar rules 
    //  2: read precedences
    //  other: skip all lines
    int state = 0;

    String line;
    while(std::getline(file, line))
    {
        /// TODO: we don't need to lex every line we should skip the comments
        TokenList tokens = LexLine(line);
        if(tokens.empty())
        {
            DeleteTokenList(tokens);
            continue;
        }

        if(tokens.at(0)->Content == "#")
        {
            DeleteTokenList(tokens);
            state++;
            continue;
        }

        switch(state)
        {
            case 1:
            AddGrammarRule(tokens);
            DeleteTokenList(tokens);
            continue;
            case 2:
            AddPrecedenceClass(tokens);
            DeleteTokenList(tokens);
            break;

            default:
            DeleteTokenList(tokens);
            continue;
        }
    }

    AssignRulePrecedences();
}




// ---------------------------------------------------------------------------------------------------------------------
// Formal grammar parser (experimental)

/// add a [newToken] to an existing [listTail]
void AddToList(ParseToken** listHead, ParseToken** listTail, ParseToken* newToken)
{
    if(*listHead == nullptr)
    {
        *listHead = newToken;
        *listTail = newToken;
        return;
    }
    else
    {
        (*listTail)->Next = newToken;
        newToken->Prev = *listTail;
        *listTail = newToken;
    }
}

/// remove the last token from ilst
void RemoveLastTokenFromList(ParseToken** listHead, ParseToken** listTail)
{
    if(listTail == nullptr)
        return;

    ParseToken* temp = *listTail;
    *listTail = (*listTail)->Prev;

    if(listTail != nullptr)
        (*listTail)->Next = nullptr;

    delete temp;
}

ParseToken* ParseTokenConstructor(String tokenType)
{
    ParseToken* gt = new ParseToken;
    gt->Next = nullptr;
    gt->Prev = nullptr;
    gt->TokenType = tokenType;
    gt->Value = nullptr;

    return gt;
}

void ParseTokenDestructor(ParseToken* token)
{
    delete token;
}

/// constructs a operation of type OperationType::Ref with either a primitive value or a named Reference stub
Operation* RefOperation(Token* refToken)
{
    Reference* ref = ReferenceForPrimitive(refToken, c_operationReferenceName);
    if(ref == nullptr)
    {
        ref = ReferenceStub(refToken->Content);
    }

    Operation* op = OperationConstructor(OperationType::Ref, ref);

    return op;
}

bool MatchGrammarPatterns(ParseToken* listHead, ParseToken* listTail, CFGRule& match)
{
    for(CFGRule* rule: Grammar)
    {
        bool isMatchForRule = true;
        ParseToken* listItr = listTail;
        for(int i=rule->IntoPattern.size()-1; i>=0; i--, listItr = listItr->Prev)
        {
            // false if rule is longer than the list
            if(listItr == nullptr)
            {
                isMatchForRule = false;
                break;
            }

            if(listItr->TokenType != rule->IntoPattern.at(i))
            {
                isMatchForRule = false;
                break;
            }
        }
        if(isMatchForRule)
        {
            match = *rule;
            return true;
        }
    }
    return false;
}

void DestroyList(ParseToken* listHead)
{
    ParseToken* prevToken;
    while(listHead != nullptr)
    {
        prevToken = listHead;
        listHead = listHead->Next;
        ParseTokenDestructor(prevToken);
    }
}


bool ParseTokenTypeMatches(String TokenType, std::vector<String> matchTypes)
{
    for(auto str: matchTypes)
    {
        if(TokenType == str)
            return true;
    }
    return false;
}


/// pushes listTail back to before the [rule] pattern and sets listSnipHead to be the head (start) of [rule] in the ParseStack
void PointToHeadOfRuleAndSnip(ParseToken** listHead, ParseToken** listTail, ParseToken** listSnipHead, CFGRule& rule)
{
    int backtrackAmount = rule.IntoPattern.size()-1;

    for(int i=0; i<backtrackAmount; i++, *listSnipHead = (*listSnipHead)->Prev);

    *listTail = (*listSnipHead)->Prev;
    
    if(*listTail != nullptr)
    {
        (*listTail)->Next = nullptr;
    }
    else
    {
        // if the remaining list is empty
        *listHead = nullptr;
    }
}

/// assumes that the list matches [rule] and removes the rule
OperationsList GetOperandsAndRemoveRule(ParseToken** listHead, ParseToken** listTail, CFGRule& rule)
{
    OperationsList operands;
    operands.reserve(5);

    ParseToken* listSnipHead = *listTail;
    
    PointToHeadOfRuleAndSnip(listHead, listTail, &listSnipHead, rule);
    
    // gets the operands
    for(ParseToken* listSnipItr = listSnipHead; listSnipItr != nullptr; listSnipItr = listSnipItr->Next)
    {
        if(ParseTokenTypeMatches(listSnipItr->TokenType, ProductionVariables))
        {
            operands.push_back(listSnipItr->Value);
        }
    }
    
    DestroyList(listSnipHead);

    return operands;
}


/// collapse a rule by adding each component as the operand of a new operation
Operation* CollapseByReduce(CFGRule& rule, OperationsList& components)
{
    return OperationConstructor(rule.OpType, components);
}

/// collapse a rule by merging all components into the first component's operand list
Operation* CollapseByMerge(CFGRule& rule, OperationsList& components)
{
    OperationsList& oplist = components.at(0)->Operands;
    
    for(size_t i=1; i< components.size(); i++)
    {
        oplist.push_back(components.at(i));
    }
    
    return components.at(0);
}

Operation* HackOperation()
{
    return OperationConstructor(OperationType::Ref, { ReferenceStub("Hack") }); 
}

Operation* CollapseByScopedEval(CFGRule& rule, OperationsList& components)
{
    if(components.size() < 3)
        components.push_back(HackOperation());

    return OperationConstructor(rule.OpType, components);
}

Operation* CollapseByUnscopedEval(CFGRule& rule, OperationsList& components)
{
    if(components.size() == 1)
    {
        auto op = components[0];
        components.clear();
        components.push_back(HackOperation());
        components.push_back(op);
        components.push_back(HackOperation());
    }
    else
    {
        auto op1 = components[0];
        auto op2 = components[1];
        
        components.clear();
        components.push_back(HackOperation());
        components.push_back(op1);
        components.push_back(op2);
    }

    return OperationConstructor(rule.OpType, components);
}

Reference* ScopeChainTerminal(Operation* op)
{
    if(op->Type != OperationType::ScopeResolution)
        return nullptr;
    
    if(op->Operands.size() == 1)
        return op->Operands.at(0)->Value;

    return op->Operands.at(1)->Value;
}

/// collapse a rule corresponding to defining a method
Operation* CollapseAsDefineMethod(CFGRule& rule, OperationsList& components)
{
    LogItDebug("Custom type", "CollapseAsDefineMethod");

    auto methodName = components.at(0)->Value->Name;
    
    ParameterList params;
    if(components.size() > 1)
    {
        if(components.at(1)->Type == OperationType::Tuple)
        {
            for(auto op: components.at(1)->Operands)
            {
                params.push_back(ScopeChainTerminal(op)->Name);
            }
        }
        else
        {
            auto chain = components.at(1);
            params.push_back(ScopeChainTerminal(chain)->Name);
        }
        DeleteOperationRecursive(components.at(1));
    }

    DeleteOperationRecursive(components.at(0));

    Reference* refToMethodObj = CreateReferenceToNewObject(methodName, BaseClass, nullptr, CurrentScope());
    ObjectOf(refToMethodObj)->Action->ParameterNames = params;
    return OperationConstructor(OperationType::DefineMethod, { OperationConstructor(OperationType::Ref, refToMethodObj) } );
}

Operation* CollapseByChain(CFGRule& rule, OperationsList& components)
{
    return OperationConstructor(rule.OpType, { components.at(0), components.at(1) } );
}

Operation* CollapseRuleInternal(CFGRule& rule, OperationsList& components)
{
    if(rule.ParseMethod == "Reduce")
    {
        return CollapseByReduce(rule, components);
    }
    else if(rule.ParseMethod == "Retain")
    {
        return components.at(0);
    }
    else if(rule.ParseMethod == "Merge")
    {
        return CollapseByMerge(rule, components);
    }
    else if(rule.ParseMethod == "Custom")
    {
        return CollapseAsDefineMethod(rule, components);
    }
    else if(rule.ParseMethod == "ScopedEval")
    {
        return CollapseByScopedEval(rule, components);
    }
    else if(rule.ParseMethod == "UnscopedEval")
    {
        return CollapseByUnscopedEval(rule, components);
    }
    else
    {
        LogIt(LogSeverityType::Sev1_Notify, "CollapseRule", "unknown collapsing procedure");
        return nullptr;
    }
}

/// reverses a rule in the ParseStack
void CollapseListByRule(ParseToken** listHead, ParseToken** listTail, CFGRule& rule)
{
    OperationsList operands = GetOperandsAndRemoveRule(listHead, listTail, rule);
    Operation* op = CollapseRuleInternal(rule, operands);

    ParseToken* t = ParseTokenConstructor(rule.FromProduction);
    t->Value = op;

    AddToList(listHead, listTail, t);
}

void LogParseStack(ParseToken* listHead)
{
    String Line;
    for(ParseToken* t = listHead; t != nullptr; t = t->Next)
    {
       Line += t->TokenType;
       Line += " ";
    }
    LogItDebug(Line, "LogParseStack");
}


/// add a new token for a Ref (object) to the ParseList
void AddRefToken(ParseToken** listHead, ParseToken** listTail, Token* token)
{
    ParseToken* t = ParseTokenConstructor("Ref");
    t->Value = RefOperation(token);
    AddToList(listHead, listTail, t);
}

/// add a new token for a simple keyword/operation to the ParseList
void AddSimpleToken(ParseToken** listHead, ParseToken** listTail, Token* token)
{
    ParseToken* t = ParseTokenConstructor(token->Content);
    AddToList(listHead, listTail, t);
}

const std::vector<String> SkippedKeyWords = { "the", "an", "a" };
const std::vector<String> ReferenceKeyWords = { "caller", "that", "it" };

void AddNextTokenToList(ParseToken** listHead, ParseToken** listTail, Token* currentToken)
{
    if(TokenMatchesType(currentToken, ObjectTokenTypes) || TokenMatchesContent(currentToken, ReferenceKeyWords))
    {
        AddRefToken(listHead, listTail, currentToken);
    }
    else
    {
        // simple tokens
        if(TokenMatchesContent(currentToken, SkippedKeyWords))
            return;

        AddSimpleToken(listHead, listTail, currentToken);
    }
}

bool CurrentRuleHasHigherPrecedence(CFGRule& rule, Token* lookaheadToken)
{
    if(lookaheadToken != nullptr)
    {
        auto symb = lookaheadToken->Content;
        if(rule.HasHigherPrecedenceClassOverride && rule.HigherPrecedenceClass.Contains(symb))
            return false;

        if(rule.HasLowerPrecedenceClassOverride && rule.LowerPrecedenceClass.Contains(symb))
            return true;
    }


    int currentRulePrecedence = rule.Precedence;
    int lookaheadPrecedence = PrecedenceOf(lookaheadToken);
    return (currentRulePrecedence >= lookaheadPrecedence);
}

/// if a grammar rule matches, and the lookahead is of less precedence, then reverse the rule and update
/// the list. continue doing so until no rules match
void TryReversingGrammarRules(ParseToken** listHead, ParseToken** listTail, Token* lookaheadToken)
{
    CFGRule match;

    while(MatchGrammarPatterns(*listHead, *listTail, match))
    {
        if(CurrentRuleHasHigherPrecedence(match, lookaheadToken))
        {
            CollapseListByRule(listHead, listTail, match);
        }
        else
        {
            break;
        }
        LogParseStack(*listHead);
    }
}

Operation* ExpressionParser(TokenList& line)
{
    ParseToken* listHead = nullptr;
    ParseToken* listTail = nullptr;

    int pos = 0;

    while(static_cast<size_t>(pos) < line.size())
    {
        Token* currentToken = line.at(pos);
        AddNextTokenToList(&listHead, &listTail, currentToken);

        LogParseStack(listHead);

        Token* lookaheadToken = nullptr;
        if(static_cast<size_t>(pos+1) < line.size())
            lookaheadToken = line.at(pos+1);

        TryReversingGrammarRules(&listHead, &listTail, lookaheadToken);

        pos++;
    }

    // if the line could not be parsed
    if(listHead != listTail)
    {
        DestroyList(listHead);
        ReportCompileMsg(SystemMessageType::Exception, "syntax error");
        FatalCompileError = true;
        return nullptr;
    }

    // resolving references will be done at runtime
    LogItDebug("end reached", "ExpressionParser");
    
    auto ast = listHead->Value;
    DestroyList(listHead);

    return ast;
}
