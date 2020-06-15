#include <string>
#include <vector>
#include <cctype>

#include "main.h"
#include "arch.h"


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
    return tokenString.at(0) == std::toupper(tokenString.at(0));
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

Token* GetToken(const std::string& line, size_t& position)
{
    Token* token;
    SkipWhiteSpace(line, position);
    std::string tokenString = "";
    
    // special case for string
    if(line.at(position) == '"')
    {
        while(++position < line.size() && line.at(position) != '"')
        {
            tokenString += line.at(position);
        }
        position++; // get rid of end quote
        token = new Token { TokenType::String, tokenString };
        return token;
    }
    for(; position < line.size() && line.at(position) != ' '; position++)
    {
        tokenString += line.at(position);    
    }

    if(tokenString == "")
        return nullptr;
    
    token = new Token { TypeOfTokenString(tokenString), tokenString};
    return token;
}


TokenList LexLine(const std::string& line)
{
    TokenList tokens;
    size_t linePosition = 0;
    while(linePosition < line.size())
    {
        Token* t = GetToken(line, linePosition);
        if(t == nullptr)
            continue;

        tokens.push_back(*t);
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
