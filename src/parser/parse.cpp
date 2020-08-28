#include "parse.h"

#include "main.h"
#include "token.h"
#include "diagnostics.h"
#include "reference.h"
#include "operation.h"
#include "executable.h"
#include "object.h"
#include "program.h"
#include "grammar.h"


std::vector<Object*> ConstantPrimitiveObjects;

// ---------------------------------------------------------------------------------------------------------------------
// Formal expression parser

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

bool TokenRefersToExistingPrimitiveObject(Token* token, Object** matchedObject)
{
    Object* obj;
    auto value = token->Content;
    bool found = false;
    switch(token->Type)
    {
        case TokenType::Integer:
        found = ListContainsPrimitiveObject(ConstantPrimitiveObjects, std::stoi(value), &obj);
        break;

        case TokenType::Boolean:
        {
            bool b = (value == "true" ? true : false);
            found = ListContainsPrimitiveObject(ConstantPrimitiveObjects, b, &obj);
        }
        break;

        case TokenType::String:
        found = ListContainsPrimitiveObject(ConstantPrimitiveObjects, value, &obj);
        break;

        case TokenType::Decimal:
        found = ListContainsPrimitiveObject(ConstantPrimitiveObjects, std::stod(value), &obj);
        break;

        default:
        return false;
    }

    *matchedObject = obj;
    return found;
}

Object* CreateObjectFromToken(Token* token)
{
    switch(token->Type)
    {
        case TokenType::Integer:
        return ObjectConstructor(IntegerClass, ObjectValueConstructor(std::stoi(token->Content)));
        
        case TokenType::Decimal:
        return ObjectConstructor(DecimalClass, ObjectValueConstructor(std::stod(token->Content)));

        case TokenType::Boolean:
        {
            bool b = (token->Content == "true" ? true : false);
            return ObjectConstructor(BooleanClass, ObjectValueConstructor(b));
        }

        case TokenType::String:
        return ObjectConstructor(StringClass, ObjectValueConstructor(token->Content));

        default:
        return nullptr;
    }
}

/// constructs a operation of type OperationType::Ref with either a primitive value or a named Reference stub
Operation* RefOperation(Token* refToken)
{
    Reference* ref = nullptr;
    Object* obj = nullptr;

    if(TokenMatchesType(refToken, PrimitiveTokenTypes))
    {
        if(!TokenRefersToExistingPrimitiveObject(refToken, &obj))
        {
            obj = CreateObjectFromToken(refToken);
            ConstantPrimitiveObjects.push_back(obj);
        }

        ref = ReferenceConstructor(c_operationReferenceName, obj);
    }
    else
    {
        ref = ReferenceStubConstructor(refToken->Content);    
    }
    
    Operation* op = OperationConstructor(OperationType::Ref, ref);

    return op;
}

bool MatchGrammarPatterns(ParseToken* listHead, ParseToken* listTail, CFGRule& match)
{
    for(CFGRule& rule: Grammar)
    {
        bool isMatchForRule = true;
        ParseToken* listItr = listTail;
        for(int i=rule.IntoPattern.size()-1; i>=0; i--, listItr = listItr->Prev)
        {
            // false if rule is longer than the list
            if(listItr == nullptr)
            {
                isMatchForRule = false;
                break;
            }

            if(listItr->TokenType != rule.IntoPattern.at(i))
            {
                isMatchForRule = false;
                break;
            }
        }
        if(isMatchForRule)
        {
            match = rule;
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
    operands.reserve(4);

    ParseToken* listSnipHead = *listTail;
    
    PointToHeadOfRuleAndSnip(listHead, listTail, &listSnipHead, rule);
    
    // gets the operands
    for(ParseToken* listSnipItr = listSnipHead; listSnipItr != nullptr; listSnipItr = listSnipItr->Next)
    {
        if(ParseTokenTypeMatches(listSnipItr->TokenType, ProductionVariables) && listSnipItr->Value != nullptr)
        {
            operands.push_back(listSnipItr->Value);
        }
    }

    DestroyList(listSnipHead);

    return operands;
}



// ---------------------------------------------------------------------------------------------------------------------
// Methods of collapsing grammar rules

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

Operation* NothingStubOperation()
{
    return OperationConstructor(OperationType::Ref, { ReferenceStubConstructor(c_nullStubName) }); 
}

/// order is caller, method, params
Operation* CollapseByUnscopedEval(CFGRule& rule, OperationsList& components)
{
    if(components.size() == 1)
    {
        auto op = components[0];
        components.clear();
        components.push_back(NothingStubOperation());
        components.push_back(op);
    }
    else
    {
        auto op1 = components[0];
        auto op2 = components[1];
        
        components.clear();
        components.push_back(NothingStubOperation());
        components.push_back(op1);
        components.push_back(op2);
    }

    return OperationConstructor(rule.OpType, components);
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
    t->Value = nullptr;
    AddToList(listHead, listTail, t);
}

const std::vector<String> SkippedKeyWords = { "the" };
const std::vector<String> ReferenceKeyWords = { "caller", "self", "that", "it" };

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

bool PositionMatchesPreprocessorRule(TokenList& list, size_t startPos, PreprocessorRule** matchedRule)
{
    PreprocessorRule* longestMatch = nullptr;
    for(auto& rule: PreprocessorRules)
    {
        if(list.size() < rule.Pattern.size() + startPos)
        {
            continue;
        }

        bool ruleMatches = true;
        for(size_t i=0; i<rule.Pattern.size(); i++)
        {
            if(rule.Pattern[i] != list[startPos + i]->Content)
            {
                ruleMatches = false;
                break;
            }
        }

        if(ruleMatches)
        {
            if(longestMatch == nullptr)
            {
                longestMatch = &rule;
            }
            else
            {
                if(longestMatch->Pattern.size() < rule.Pattern.size())
                {
                    longestMatch = &rule;
                }
            }
        }
    }

    if(longestMatch != nullptr)
    {
        *matchedRule = longestMatch;
        return true;
    }

    return false;
}

TokenList Preprocess(TokenList& list)
{
    TokenList processedList;
    int newListIndex = 0;
    for(size_t i=0; i<list.size(); i++)
    {
        PreprocessorRule* rule = nullptr;
        if(PositionMatchesPreprocessorRule(list, i, &rule))
        {
            Token* t = new Token;
            *t = { TokenType::Simple, rule->Becomes, newListIndex };
            processedList.push_back(t);
            i += rule->Pattern.size() - 1;
        }
        else
        {
            auto t = list[i];
            t->Position = newListIndex;
            processedList.push_back(t);
        }

        newListIndex++;
    }

    return processedList;
}


Operation* CompilationErrorFound(String error = "syntax error")
{
    LogIt(LogSeverityType::Sev3_Critical, "ERROR", error);
    ReportCompileMsg(SystemMessageType::Exception, error);
    FatalCompileError = true;
    return nullptr;
}

/// Checks for errors programmers may make and given better suggestions (TODO: make message )
bool PassesInitialCheckForLineErrors(TokenList* tokensPtr)
{
    for(size_t i = 0; i < tokensPtr->size() - 1; i++)
    {
        if(tokensPtr->at(i)->Type == TokenType::Simple && tokensPtr->at(i + 1)->Type == TokenType::Simple)
        {
            String errorMessage = "Cannot have multiple operators next to each other " + tokensPtr->at(i)->Content + tokensPtr->at(i + 1)->Content;
            LogDiagnostics(tokensPtr, errorMessage);
            CompilationErrorFound(errorMessage);
            return false;
        }
    }
    // determines if the first or last token is an operator (with -#'s being the exception at index 0)
    if((tokensPtr->at(0)->Type == TokenType::Simple && tokensPtr->at(0)->Content.find("-") != 0) || tokensPtr->at(tokensPtr->size() - 1)->Type == TokenType::Simple)
    {
        String errorMessage = "Unfinished expression " + tokensPtr->at(0)->Content;
        LogDiagnostics(tokensPtr, errorMessage);
        CompilationErrorFound(errorMessage);
        return false;
    }
    return true;
}

Operation* ExpressionParser(TokenList& rawline)
{
    ParseToken* listHead = nullptr;
    ParseToken* listTail = nullptr;

    TokenList line = Preprocess(rawline);
    LogDiagnostics(line);
    if(!PassesInitialCheckForLineErrors(&line))
    {
        return nullptr;
    }

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
        return CompilationErrorFound();
    }

    // resolving references will be done at runtime
    auto ast = listHead->Value;
    DestroyList(listHead);

    return ast;
}

void InitParser()
{
    ConstantPrimitiveObjects.clear();
    ConstantPrimitiveObjects.reserve(64);
}