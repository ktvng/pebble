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
    else if(Name=="Return")
        return OperationType::Return;

    else
        return OperationType::Ref;
}


std::vector<CFGRule*> Grammar;
std::list<PrecedenceClass> PrecedenceRules;
std::vector<String> ProductionVariables = { "Ref" };

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
    LogIt(Sev3_Critical, "PrecedenceOf", MSG("unknown operation symbol %s", opSymbol));
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

void AddGrammarRule(TokenList& tokens, CFGRule** rule)
{
    if(tokens.at(0)->Content == "@")
    {
        *rule = new CFGRule;
        (*rule)->Name = tokens.at(1)->Content;
        (*rule)->Symbol = tokens.at(2)->Content;
        (*rule)->ParseOperation = tokens.at(3)->Content;
        (*rule)->OpType = StringNameToOperationType((*rule)->Name);

    }
    else if(tokens.at(0)->Type == TokenType::Reference)
    {
        (*rule)->FromProduction = tokens.at(0)->Content;
        AddProductionVariable((*rule)->FromProduction);
        for(size_t i=3; i<tokens.size(); i++)
        {
            (*rule)->IntoPattern.push_back(tokens.at(i)->Content);
        }
        Grammar.push_back((*rule));
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
            case 1:
            AddGrammarRule(tokens, &rule);
            continue;
            case 2:
            AddPrecedenceClass(tokens);
            break;

            default:
            continue;
        }
    }

    AssignRulePrecedences();
}




// ---------------------------------------------------------------------------------------------------------------------
// Formal grammar parser (experimental)
struct ParseToken
{
    String TokenType;
    Operation* Value;
    ParseToken* Next = nullptr;
    ParseToken* Prev = nullptr;
}; 


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

ParseToken* ParseTokenConstructor(String tokenType)
{
    ParseToken* gt = new ParseToken;
    gt->Next = nullptr;
    gt->Prev = nullptr;
    gt->TokenType = tokenType;
    gt->Value = nullptr;

    return gt;
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

CFGRule* MatchGrammarPatterns(ParseToken* listHead, ParseToken* listTail)
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
            return rule;
    }
    return nullptr;
}

void DestroyList(ParseToken* listHead)
{
    ParseToken* prevToken;
    while(listHead != nullptr)
    {
        prevToken = listHead;
        listHead = listHead->Next;
        delete prevToken;
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
void PointToHeadOfRuleAndSnip(ParseToken** listHead, ParseToken** listTail, ParseToken** listSnipHead, CFGRule* rule)
{
    int backtrackAmount = rule->IntoPattern.size()-1;

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
OperationsList GetOperandsAndRemoveRule(ParseToken** listHead, ParseToken** listTail, CFGRule* rule)
{
    OperationsList operands = {};
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
Operation* CollapseByReduce(CFGRule* rule, OperationsList& components)
{
    return OperationConstructor(rule->OpType, components);
}

/// collapse a rule by merging component operands into the same list
Operation* CollapseByMerge(CFGRule* rule, OperationsList& components)
{
    OperationsList mergedOperands;
    for(auto op: components)
    {
        // direcly merge ::Ref type operations into the new operandsList
        if(op->Type == OperationType::Ref)
            mergedOperands.push_back(op);
        // for all other operation types, port the operands directly into the new operandsList
        else
        {
            for(auto opOperand: op->Operands)
                mergedOperands.push_back(opOperand);
        }
    }

    return OperationConstructor(rule->OpType, mergedOperands);
}

/// collapse a rule corresponding to defining a method
Operation* CollapseAsDefineMethod(CFGRule* rule, OperationsList& components)
{
    LogItDebug("Custom type", "CollapseListByRule");

        auto methodName = components.at(0)->Value->Name;
        
        Method* m = MethodConstructor(CurrentScope());
        EnterScope(m->Parameters);
        {
            if(components.size() > 1)
            {
                // if the operand is already a return operation
                if(components.at(1)->Type == OperationType::Ref)
                    NullReference(components.at(1)->Value->Name);
                // if the operand is a list of parameters
                else
                    for(size_t i=0; i<components.at(1)->Operands.size(); i++)
                        NullReference(components.at(1)->Operands.at(i)->Value->Name);
            }
        }
        ExitScope();
        
        Reference* ref = ReferenceFor(methodName, m);
        return OperationConstructor(OperationType::DefineMethod, { OperationConstructor(OperationType::Ref, ref) } );
}


Operation* CollapseRuleInternal(CFGRule* rule, OperationsList& components)
{
    if(rule->ParseOperation == "Reduce")
    {
        return CollapseByReduce(rule, components);
    }
    else if(rule->ParseOperation == "Retain")
    {
        return components.at(0);
    }
    else if(rule->ParseOperation == "Merge")
    {
        return CollapseByMerge(rule, components);
    }
    else if(rule->ParseOperation == "Custom")
    {
        return CollapseAsDefineMethod(rule, components);
    }
    else
    {
        LogIt(LogSeverityType::Sev1_Notify, "CollapseRule", "unknown collapsing procedure");
        return nullptr;
    }
}

/// reverses a rule in the ParseStack
void CollapseListByRule(ParseToken** listHead, ParseToken** listTail, CFGRule* rule)
{
    OperationsList operands = GetOperandsAndRemoveRule(listHead, listTail, rule);
    Operation* op = CollapseRuleInternal(rule, operands);

    ParseToken* t = ParseTokenConstructor(rule->FromProduction);
    t->Value = op;

    AddToList(listHead, listTail, t);
}

/// if [op] is assigning to an undefined reference, define it
void DefineNewReferenceIfNecessary(Operation* op)
{
    if(op->Type == OperationType::Assign)
    {
        Reference* stub = op->Operands.at(0)->Value;
        auto assignToRef = ReferenceFor(stub->Name);

        // creates define operation
        if(assignToRef == nullptr)
        {
            Operation* newRefReturn = OperationConstructor(OperationType::Ref, NullReference(stub->Name));
            Operation* defRef = OperationConstructor(OperationType::Define, { newRefReturn });
            op->Operands[0] = defRef;

            delete stub;
        }
    }
}

void ResolveRefOperation(Operation* refOp)
{
    Reference* stub = refOp->Value;

    // verify that Reference is in fact a stub 
    if(IsReferenceStub(stub))
    {
        // get the reference by name which should be defined
        auto resolvedRef = ReferenceFor(stub->Name);
        
        if(resolvedRef == nullptr)
        {
            ReportCompileMsg(SystemMessageType::Exception, MSG("cannot resolve reference %s. make sure it has been defined", stub->Name));
            resolvedRef = NullReference(stub->Name);
        }
        refOp->Value = resolvedRef;
    }
}

/// recursively resolves all reference stubs in [op]
void ResolveReferences(Operation* op)
{
    // breaks recursion. these types are handled in the parent
    if(op->Type == OperationType::Ref)
        return;

    if(op->Type == OperationType::DefineMethod)
        return;

    DefineNewReferenceIfNecessary(op);

    for(auto operand: op->Operands)
    {
        if(operand->Type == OperationType::Ref)
        {
            ResolveRefOperation(operand);
        }
        else
        {
            ResolveReferences(operand);
        }
    }
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

/// if a grammar rule matches, and the lookahead is of less precedence, then reverse the rule and update
/// the list. continue doing so until no rules match
void TryMatchingGrammarRules(ParseToken** listHead, ParseToken** listTail, Token* lookaheadToken)
{
    CFGRule* match = MatchGrammarPatterns(*listHead, *listTail);

    while(match != nullptr)
    {
        if(match != nullptr){
            int currentRulePrecedence = match->Precedence;
            int lookaheadPrecedence = PrecedenceOf(lookaheadToken);

            if(currentRulePrecedence >= lookaheadPrecedence)
            {
                CollapseListByRule(listHead, listTail, match);
                match = MatchGrammarPatterns(*listHead, *listTail);
            }
            else
            {
                break;
            }
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

        if(TokenMatchesType(line.at(pos), ObjectTokenTypes))
        {
            AddRefToken(&listHead, &listTail, currentToken);
        }
        else
        {
            AddSimpleToken(&listHead, &listTail, currentToken);
        }

        LogParseStack(listHead);

        Token* lookaheadToken = nullptr;
        if(static_cast<size_t>(pos+1) < line.size())
            lookaheadToken = line.at(pos+1);

        TryMatchingGrammarRules(&listHead, &listTail, lookaheadToken);

        pos++;
    }
    // if the line could not be parsed
    if(listHead != listTail)
    {
        ReportCompileMsg(SystemMessageType::Exception, "bad format");
    }

    ResolveReferences(listHead->Value);
    
    LogItDebug("end reached", "ExpressionParser");
    return listHead->Value;
}