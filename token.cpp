#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include <fstream>
#include <list>

#include "main.h"
#include "arch.h"
#include "diagnostics.h"
#include "reference.h"
#include "operation.h"

void SkipWhiteSpace(const std::string& line, size_t& position)
{
    for(; position < line.size() && line.at(position) == ' '; position++);
}

bool IsInteger(const std::string& tokenString)
{
    for(size_t i=0; i<tokenString.size(); i++)
    {
        if(!std::isdigit(tokenString.at(i)))
            return false;
    }
    return true;
}

bool IsDecimal(const std::string& tokenString)
{
    bool foundDecimalPointAlready = false;
    for(size_t i=0; i<tokenString.size(); i++)
    {
        if(!std::isdigit(tokenString.at(i)))
        {
            if(tokenString.at(i) == '.')
            {
                if(foundDecimalPointAlready)
                    return false;
                foundDecimalPointAlready = true;
            }
            else
            {
                return false;
            }
            
        }
    }
    return true;
}

bool IsString(const std::string& tokenString)
{
    return tokenString.at(0) == '"' && tokenString.at(tokenString.size()-1) == '"';
}

std::string ToLowerCase(const std::string& str)
{
    std::string lowerCaseStr = "";
    for(size_t i=0; i<str.size(); i++)
    {
        lowerCaseStr += std::tolower(str.at(i));
    }
    return lowerCaseStr;
}

bool IsBoolean(const std::string& tokenString)
{
    return ToLowerCase(tokenString) == "true" || ToLowerCase(tokenString) == "false";
}

bool IsReference(const std::string& tokenString)
{
    return (tokenString.at(0) >= 65 && tokenString.at(0) <= 90);
}

TokenType TypeOfTokenString(const std::string& tokenString)
{
    if(IsInteger(tokenString))
    {
        return TokenType::Integer;
    }
    else if(IsDecimal(tokenString))
    {
        return TokenType::Decimal;
    }
    else if(IsString(tokenString))
    {
        return TokenType::String;
    }
    else if(IsBoolean(tokenString))
    {
        return TokenType::Boolean;
    }
    else if(IsReference(tokenString))
    {
        return TokenType::Reference;
    }
    else
    {
        return TokenType::Simple;
    }
    
}

const String StringStartChars = "\"'";
const char DecimalPoint = '.';
const String SingletonChars = "!@#$%^*()-+=[]{}\\:;<>,./?~`&|";
const String DoubledChars = "&&||==!=";

bool IsSingleTonChar(char c)
{
    for(size_t i=0; i<SingletonChars.size(); i++)
        if(c == SingletonChars.at(i))
            return true;

    return false;
}

Token* GetSingletonCharToken(const String& line, size_t& position, int tokenNumber)
{
    String tokenString = "";
    tokenString += line.at(position++);
    return new Token { TokenType::Simple, tokenString, tokenNumber };
}

bool IsDoubleCharToken(size_t& position, const String& line)
{
    if(position + 1 >= line.size())
        return false;
    
    for(size_t i=0; i<DoubledChars.size(); i+=2)
    {
        if(line.at(position) == DoubledChars.at(i) && line.at(position+1) == DoubledChars.at(i+1))
            return true;
    }

    return false;
}

Token* GetDoubleCharToken(const String& line, size_t& position, int tokenNumber)
{
    String tokenString = "";
    tokenString += line.at(position);
    tokenString += line.at(position+1);
    position += 2;
    
    return new Token { TokenType::Simple, tokenString, tokenNumber };
}

bool IsStringStartChar(char c)
{
    return c == '"';
}

Token* GetStringToken(const String& line, size_t& position, int tokenNumber)
{
    String tokenString;
    while(++position < line.size() && line.at(position) != '"')
    {
        tokenString += line.at(position);
    }
    position++; // get rid of end quote
    return new Token { TokenType::String, tokenString, tokenNumber };
}


bool IsNumericChar(char c)
{
    return 48 <= static_cast<int>(c) && 57 >= static_cast<int>(c);
}

bool IsActuallyDecimalPoint(const size_t& position, const String& line)
{
    return (position > 0 && IsNumericChar(line.at(position-1))) &&
        (position + 1 < line.size() && IsNumericChar(line.at(position+1)));
}

Token* GetToken(const std::string& line, size_t& position, int tokenNumber)
{

    SkipWhiteSpace(line, position);
    if(position >= line.size())
        return nullptr;


    // special case for string
    if(IsStringStartChar(line.at(position)))
    {
        return GetStringToken(line, position, tokenNumber);
    }
    else if(IsDoubleCharToken(position, line))
    {
        return GetDoubleCharToken(line, position, tokenNumber);
    }
    else if(IsSingleTonChar(line.at(position)))
    {
        return GetSingletonCharToken(line, position, tokenNumber);
    }

    Token* token;
    std::string tokenString = "";

    for(; position < line.size() && line.at(position) != ' '; position++)
    {
        if(line.at(position) != DecimalPoint && IsSingleTonChar(line.at(position)))
            break;
        else if(line.at(position) == DecimalPoint)
        {
            if(!IsActuallyDecimalPoint(position, line))
                break;
        }
        tokenString += line.at(position);    
    }

    if(tokenString == "")
        return nullptr;
    
    token = new Token { TypeOfTokenString(tokenString), tokenString, tokenNumber };

    return token;
}


TokenList LexLine(const std::string& line)
{
    TokenList tokens;
    size_t linePosition = 0;
    int tokenNumber = 0;
    while(linePosition < line.size())
    {
        Token* t = GetToken(line, linePosition, tokenNumber);
        if(t == nullptr)
            continue;

        tokens.push_back(t);
        tokenNumber++;
    }

    return tokens;
}

std::string GetStringTokenType(TokenType type)
{
    std::string typeString;
    switch(type)
    {
        case TokenType::Boolean:
        typeString = "boolean  ";
        break;

        case TokenType::Decimal:
        typeString = "decimal  ";
        break;

        case TokenType::Integer:
        typeString = "integer  ";
        break;

        case TokenType::Reference:
        typeString = "reference";
        break;

        case TokenType::Simple:
        typeString = "simple   ";
        break;

        case TokenType::String:
        typeString = "string   ";
        break;

        default:
        typeString = "unknown  ";
        break;
    }

    return typeString;
}

bool SameLetterOrChar(char c1, char c2)
{
    return std::toupper(c1) == std::toupper(c2);
}

bool StringCaseInsensitiveEquals(const std::string& str1, const std::string& str2)
{
    if(str1.size() != str2.size())
        return false;

    for(size_t pos=0; pos<str1.size(); pos++)
    {
        if(!SameLetterOrChar(str1.at(pos), str2.at(pos)))
            return false;
    }
    
    return true;
}

Token* FindToken(const TokenList& tokens, std::string str)
{
    for(Token* t: tokens)
    {
        if(StringCaseInsensitiveEquals(str, t->Content))
            return t;
    }
    return nullptr;
}

void RenumberTokenList(TokenList& tokens)
{
    for(int i=0; static_cast<size_t>(i)<tokens.size(); i++)
    {
        tokens.at(i)->Position = i;
    }
}



TokenList RightOfToken(const TokenList& tokens, Token* pivotToken)
{
    TokenList rightList;
    rightList.reserve(tokens.size());

    for(int i=pivotToken->Position + 1; static_cast<size_t>(i)<tokens.size(); i++)
        rightList.push_back(tokens.at(i));
    
    RenumberTokenList(rightList);
    return rightList;
}

TokenList LeftOfToken(const TokenList& tokens, Token* pivotToken)
{
    TokenList leftList;
    leftList.reserve(tokens.size());

    for(int i=0; i<pivotToken->Position; i++)
        leftList.push_back(tokens.at(i));
    
    RenumberTokenList(leftList);
    return leftList;
}


bool TokenMatchesType(Token* token, std::vector<TokenType> types)
{
    for(TokenType type: types)
    {
        if(type == token->Type)
            return true;
    }
    return false;
}

bool TokenMatchesType(Token* token, TokenType type)
{
    std::vector<TokenType> types = { type };
    return TokenMatchesType(token, types);
}

bool TokenMatchesContent(Token* token, std::vector<String> contents)
{
    for(String content: contents)
    {
        if(content == token->Content)
            return true;
    }
    return false;
}

bool TokenMatchesContent(Token* token, String content)
{
    std::vector<String> contents = { content };
    return TokenMatchesContent(token, contents);
}

Token* NextTokenMatching(const TokenList& tokens, std::vector<TokenType> types, int& pos)
{
    for(; static_cast<size_t>(pos)<tokens.size(); pos++)
    {
        if(TokenMatchesType(tokens.at(pos), types))
            return tokens.at(pos++);
    }
    pos = -1;
    return nullptr;
}


Token* NextTokenMatching(const TokenList& tokens, TokenType type, int& pos)
{
    std::vector<TokenType> types = { type };
    return NextTokenMatching(tokens, types, pos);
}

Token* NextTokenMatching(const TokenList& tokens, TokenType type)
{
    int i = 0;
    return NextTokenMatching(tokens, type, i);
}

Token* NextTokenMatching(const TokenList& tokens, std::vector<TokenType> types)
{
    int i = 0;
    return NextTokenMatching(tokens, types, i);
}

Token* NextTokenMatching(const TokenList& tokens, std::vector<String> contents, int& pos)
{
    for(; static_cast<size_t>(pos)<tokens.size(); pos++)
    {
        if(TokenMatchesContent(tokens.at(pos), contents))
            return tokens.at(pos++);
    }
    pos = -1;
    return nullptr;
}

Token* NextTokenMatching(const TokenList& tokens, std::vector<String> contents)
{
    int pos = 0;
    return NextTokenMatching(tokens, contents, pos);
}

Token* NextTokenMatching(const TokenList& tokens, String content, int& pos)
{
    std::vector<String> contents = { content };
    return NextTokenMatching(tokens, contents, pos);
}

Token* NextTokenMatching(const TokenList& tokens, String content)
{
    int pos = 0;
    return NextTokenMatching(tokens, content, pos);
}


bool TokenListContainsContent(const TokenList& tokenList, std::vector<String> contents)
{
    for(Token* t: tokenList)
    {
        if(TokenMatchesContent(t, contents))
            return true;
    }
    return false;
}


// ---------------------------------------------------------------------------------------------------------------------
// Formal grammar parser parser (experimental)
struct CFGRule
{
    String Name;
    String Symbol;
    int Precedence;
    OperationType OpType;
    std::vector<String> IntoPattern;
    String FromProduction;
    String ParseOperation;
};

struct PrecedenceClass
{
    std::vector<String> Members;
};

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
    else if(Name=="IsGreaterThan")
        return OperationType::IsGreaterThan;
    else if(Name=="IsLessThan")
        return OperationType::IsLessThan;


    else if(Name=="Evaluate")
        return OperationType::Evaluate;
    else if(Name=="If")
        return OperationType::If;
    else if(Name=="DefineMethod")
        return OperationType::DefineMethod;
    else if(Name=="Assign")
        return OperationType::Assign;
    else if(Name=="Define")
        return OperationType::Define;
    else if(Name=="Param")
        return OperationType::Evaluate;
    else if(Name=="Print")
        return OperationType::Print;

    else
        return OperationType::Ref;
}


std::vector<CFGRule*> Grammar;
std::list<PrecedenceClass> PrecedenceRules;

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
    LogIt(Sev3_Critical, "PrecedenceOf", MSG("unknown operation symbol %s", opSymbol));
    return 0;
}

void AssignRulePrecedences()
{
    for(auto rule: Grammar)
    {
        rule->Precedence = PrecedenceOf(rule->Symbol);
    }
}

void CompileGrammar()
{
    std::fstream file;
    file.open(".\\grammar.txt", std::ios::in);


    CFGRule* rule = nullptr;

    // state: +1 upon every occurance of '###'
    //  0: skip all lines
    //  1: read grammar rules 
    //  2: read precedences
    //  other: skip all lines
    int state = 0;

    String line;
    while(std::getline(file, line))
    {
        TokenList tokens = LexLine(line);
        if(tokens.empty())
            continue;

        if(tokens.at(0)->Content == "#")
        {
            state++;
            continue;
        }

        switch(state)
        {
            case 0:
            continue;

            case 1:
            if(tokens.at(0)->Content == "@")
            {
                rule = new CFGRule;
                rule->Name = tokens.at(1)->Content;
                rule->Symbol = tokens.at(2)->Content;
                rule->ParseOperation = tokens.at(3)->Content;
                rule->OpType = StringNameToOperationType(rule->Name);
            }
            else if(tokens.at(0)->Type == TokenType::Reference)
            {
                rule->FromProduction = tokens.at(0)->Content;
                for(size_t i=3; i<tokens.size(); i++)
                {
                    rule->IntoPattern.push_back(tokens.at(i)->Content);
                }
                Grammar.push_back(rule);
            }
            continue;
            case 2:
            {
                PrecedenceClass precdence;
                for(Token* t: tokens)
                {
                    precdence.Members.push_back(t->Content);
                }
                PrecedenceRules.push_front(precdence);
            }


            default:
            continue;
        }
    }

    AssignRulePrecedences();
}




// ---------------------------------------------------------------------------------------------------------------------
// Formal grammar parser (experimental)
struct GrammarToken
{
    String TokenType;
    Operation* Value;
    GrammarToken* Next = nullptr;
    GrammarToken* Prev = nullptr;
}; 



void AddToList(GrammarToken** listHead, GrammarToken** listTail, GrammarToken* newToken)
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

GrammarToken* GrammarTokenConstructor(String tokenType)
{
    GrammarToken* gt = new GrammarToken;
    gt->Next = nullptr;
    gt->Prev = nullptr;
    gt->TokenType = tokenType;
    gt->Value = nullptr;

    return gt;
}

Operation* ReturnOperation(Token* refToken)
{
    Reference* ref = ReferenceForPrimitive(refToken, c_operationReferenceName);
    if(ref == nullptr)
    {
        ref = ReferenceStub(refToken->Content);
    }
    Operation* op = OperationConstructor(OperationType::Ref, ref);

    return op;
}

// Operation* NewReferenceReturn(Token* refToken)
// {
//     Reference* ref = NullReference(refToken->Content);
//     Operation* op = OperationConstructor(OperationType::Ref, ref);

//     return op;
// }

CFGRule* MatchGrammarPatterns(GrammarToken* listHead, GrammarToken* listTail)
{
    for(CFGRule* rule: Grammar)
    {
        bool isMatchForRule = true;
        GrammarToken* listItr = listTail;
        for(int i=rule->IntoPattern.size()-1; i>=0; i--, listItr = listItr->Prev)
        {
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
            return rule;
    }
    return nullptr;
}

void DestroyList(GrammarToken* listHead)
{
    // TODO: free memoery
}

// TODO
std::vector<String> ProductionRules()
{
    std::vector<String> productions;
    for(auto rule: Grammar)
    {
        productions.push_back(rule->FromProduction);
    }
    productions.push_back("Ref");
    return productions;
}

bool GrammarTokenTypeMatches(String TokenType, std::vector<String> matchTypes)
{
    for(auto str: matchTypes)
    {
        if(TokenType == str)
            return true;
    }
    return false;
}

/// assumes that the list matches [rule]
OperationsList GetOperandsAndRemoveRule(GrammarToken** listHead, GrammarToken** listTail, CFGRule* rule)
{
    OperationsList operands = {};
    int backtrackAmount = rule->IntoPattern.size()-1;

    GrammarToken* listSnipHead = *listTail;

    for(int i=0; i<backtrackAmount; i++, listSnipHead = listSnipHead->Prev);

    *listTail = listSnipHead->Prev;
    
    if(*listTail != nullptr)
    {
        (*listTail)->Next = nullptr;
    }
    else
    {
        *listHead = nullptr;
    }
    
    
    for(GrammarToken* listSnipItr = listSnipHead; listSnipItr != nullptr; listSnipItr = listSnipItr->Next)
    {
        if(GrammarTokenTypeMatches(listSnipItr->TokenType, ProductionRules())){
            operands.push_back(listSnipItr->Value);
        }
    }
    
    DestroyList(listSnipHead);

    return operands;
}

void ReduceList(GrammarToken** listHead, GrammarToken** listTail, CFGRule* rule)
{
    OperationsList operands = GetOperandsAndRemoveRule(listHead, listTail, rule);
    Operation* op;

    if(rule->ParseOperation == "Reduce")
    {
        op = OperationConstructor(rule->OpType, operands);
    }
    else if(rule->ParseOperation == "Retain"){
        op = operands.at(0);
    }
    else if(rule->ParseOperation == "Merge")
    {
        // TODO
        OperationsList mergedOperands;
        for(auto op: operands)
        {
            if(op->Type == OperationType::Ref)
                mergedOperands.push_back(op);
            else
            {
                for(auto opOperand: op->Operands)
                    mergedOperands.push_back(opOperand);
            }
        }

        op = OperationConstructor(rule->OpType, mergedOperands);
    }
    else if(rule->ParseOperation == "Custom")
    {
        LogItDebug("Custom type", "ReduceList");
        // TODO: this is a hotfix taken kinda from operantion.cpp
        String methodName = operands.at(0)->Value->Name;
        
        Method* m = new Method;
        m->Parameters = ScopeConstructor(PROGRAM->GlobalScope);
        m->CodeBlock = BlockConstructor(m->Parameters);
        
        if(operands.size() > 1)
            for(size_t i=0; i<operands.at(1)->Operands.size(); i++)
                m->Parameters->ReferencesIndex.push_back(NullReference(operands.at(1)->Operands.at(i)->Value->Name));
        
        Reference* ref = ReferenceFor(methodName, m);
        op = OperationConstructor(OperationType::DefineMethod, { OperationConstructor(OperationType::Ref, ref) } );

        // TODO: clean up memory

    }

    GrammarToken* t = GrammarTokenConstructor(rule->FromProduction);
    t->Value = op;

    AddToList(listHead, listTail, t);
}

void ResolveReferences(Operation* op)
{
    if(op->Type == OperationType::Ref)
        return;

    // generic prodecure
    if(op->Type == OperationType::Assign)
    {
        Reference* stub = op->Operands.at(0)->Value;
        Reference* assignToRef = ReferenceFor(stub->Name);

        if(assignToRef == nullptr)
        {
            Operation* newRef = OperationConstructor(OperationType::Ref, NullReference(stub->Name));
            Operation* defRef = OperationConstructor(OperationType::Define, { newRef });
            op->Operands[0] = defRef;

            // TODO: cleanup
        }
    }

    for(auto operand: op->Operands)
    {
        if(operand->Type == OperationType::Ref)
        {
            Reference* stub = operand->Value;
        
            // verify that Reference is in fact a stub 
            if(stub->ToObject == nullptr && stub->ToMethod == nullptr)
            {
                // this will add the reference to scope
                Reference* resolvedRef = ReferenceFor(stub->Name);
                
                if(resolvedRef == nullptr)
                {
                    ReportCompileMsg(SystemMessageType::Exception, MSG("cannot resolve reference %s", stub->Name));
                    resolvedRef = NullReference(stub->Name);
                }

                operand->Value = resolvedRef;
            }
        }
        else
        {
            ResolveReferences(operand);
        }
    }
}

Operation* ExpressionParser(TokenList& line)
{
    GrammarToken* listHead = nullptr;
    GrammarToken* listTail = nullptr;

    int pos = 0;

    LogDiagnostics(line);

    while(static_cast<size_t>(pos) < line.size())
    {
        if(TokenMatchesType(line.at(pos), ObjectTokenTypes))
        {
            LogItDebug("added new GrammarToken for ref", "ExpressionParser");
            
            GrammarToken* t = GrammarTokenConstructor("Ref");
            t->Value = ReturnOperation(line.at(pos));
            AddToList(&listHead, &listTail, t);
        }
        else
        {
            LogItDebug("added new GrammarToken for symbol", "ExpressionParser");
            GrammarToken* t = GrammarTokenConstructor(line.at(pos)->Content);
            AddToList(&listHead, &listTail, t);
        }

        for(GrammarToken* t = listHead; t != nullptr; t = t->Next)
            std::cout << t->TokenType << " ";
        std::cout << std::endl;

        CFGRule* match = MatchGrammarPatterns(listHead, listTail);

        while(match != nullptr)
        {
            LogItDebug(MSG("matched grammar %s", match->Name), "ExpressionParser");
            if(match != nullptr){
                int currentRulePrecedence = match->Precedence;

                int nextRulePrecedence = -1;

                // lookahead
                if(static_cast<size_t>(pos+1) < line.size() && line.at(pos+1))
                    nextRulePrecedence = PrecedenceOf(line.at(pos+1)->Content);

                if(currentRulePrecedence >= nextRulePrecedence)
                {
                    LogItDebug("reducing", "ExpressiongParser");
                    ReduceList(&listHead, &listTail, match);
                    LogItDebug("reduce finished", "ExpressionParser");
                    match = MatchGrammarPatterns(listHead, listTail);
                }
                else
                {
                    break;
                }
            }
            for(GrammarToken* t = listHead; t != nullptr; t = t->Next)
                std::cout << t->TokenType << " ";
            std::cout << std::endl;
        }
        pos++;
    }
    if(listHead != listTail)
    {
        ReportCompileMsg(SystemMessageType::Exception, "bad format");
    }

    LogItDebug("resolving references", "ExpressionParser");
    ResolveReferences(listHead->Value);
    
    LogItDebug("end reached", "ExpressionParser");
    LogDiagnostics(listHead->Value);
    return listHead->Value;
}