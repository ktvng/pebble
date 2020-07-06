#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include <fstream>

#include "main.h"
#include "diagnostics.h"
#include "reference.h"
#include "operation.h"


// ---------------------------------------------------------------------------------------------------------------------
// TODO:
// 1. Match boolean tokens in any case (i.e. TRUE, FaLsE)


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
const String DoubledChars = "&&||==!=>=<=" "'s";

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
        if(line.at(position) != DecimalPoint && (IsSingleTonChar(line.at(position)) || IsDoubleCharToken(position, line)))
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
