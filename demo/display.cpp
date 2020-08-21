#include <iostream>
#include <regex>
#include "display.h"
#include "consolecolor.h"
#include "program.h"
#include "executable.h"
#include "token.h"

const std::string IndentStr = "    ";
const char* OPERATOR_COLOR = CONSOLE_BLUE;
const char* METHOD_COLOR = CONSOLE_CYAN;
const char* CALL_COLOR = CONSOLE_WHITE;
const char* CONTROL_COLOR = CONSOLE_BLUE;
const char* SYS_COLOR = CONSOLE_RED;
const char* STRING_COLOR = CONSOLE_MAGENTA;
const char* NUMBER_COLOR = CONSOLE_GREEN;
const char* LITERAL_COLOR = CONSOLE_BLUE;
const char* SPECIAL_CALL_COLOR = CONSOLE_YELLOW;
const char* BIND_TYPE_COLOR = CONSOLE_GREEN;


const char* GetSimpleTokenColor(const Token* token)
{
    auto content = token->Content;
    
    std::regex typeBinding("\\ba");
    if(std::regex_search(content, typeBinding))
        return SYS_COLOR;

    std::regex supportFunctionRx("\\b(let|ask|print|say|consider|take|here|inherits)\\b");
    if(std::regex_search(content, supportFunctionRx))
        return SYS_COLOR;

    std::regex operatorComparison("<(?!<|=)|>(?!<|=|>)|<=|>=|==|!=");
    if(std::regex_search(content, operatorComparison))
        return OPERATOR_COLOR;

    std::regex operatorAssignment("(=|\\bis\\b)");
    if(std::regex_search(content, operatorAssignment))
        return OPERATOR_COLOR;

    std::regex operatorScope("'s");
    if(std::regex_search(content, operatorScope))
        return OPERATOR_COLOR;

    std::regex operatorArithmetic("(%|\\*|\\+|\\-|/)");
    if(std::regex_search(content, operatorArithmetic))
        return OPERATOR_COLOR;

    std::regex constLanguage("\\b(true|false)\\b");
    if(std::regex_search(content, constLanguage))
        return LITERAL_COLOR;

    std::regex referenceLanguage("\\b(it|that|caller|self)\\b");
    if(std::regex_search(content, referenceLanguage))
        return SPECIAL_CALL_COLOR;

    std::regex operatorLogical("\\b(and|&&|or|is|not)\\b");
    if(std::regex_search(content, operatorLogical))
        return OPERATOR_COLOR;

    std::regex controlOperator("\\b(if|else|while|for each|return)\\b");
    if(std::regex_search(content, controlOperator))
        return CONTROL_COLOR;

    return CONSOLE_WHITE;   
}

const char* GetTokenColor(const Token* token, const Token* prevToken, const Token* nextToken)
{
    switch(token->Type)
    {
        case TokenType::Boolean:
            return LITERAL_COLOR;
        
        case TokenType::Decimal:
        case TokenType::Integer:
            return NUMBER_COLOR;

        case TokenType::String:
            return STRING_COLOR;

        case TokenType::Reference:
        {
            if(prevToken != nullptr && (prevToken->Content == "a" || prevToken->Content == "an"))
            {
                return BIND_TYPE_COLOR;
            }
            if(nextToken != nullptr && (nextToken->Content == "(" || nextToken->Content == ":"))
            {
                return METHOD_COLOR;
            }
            else
            {
                return CALL_COLOR;
            }
        }

        case TokenType::Simple:
            break;
    }

    // token must be TokenType::Simple here
    return GetSimpleTokenColor(token);
}

std::string Formatted(const Token* token)
{
    switch(token->Type)
    {
        case TokenType::String:
            return "\"" + token->Content + "\"";
        
        default:
            return token->Content;
    }
}

std::string IfNeededAddSpace(const Token* token, const Token* lastToken)
{
    std::string space = " ";
    std::string noSpace = "";

    if(lastToken == nullptr)
        return noSpace;

    if(lastToken->Type == TokenType::Reference && token->Content == "(")
        return noSpace;

    if(token->Content == ",")
        return noSpace;

    if(lastToken->Content == "(")
        return noSpace;

    if(token->Content == ")")
        return noSpace;

    if(token->Content == ":")
        return noSpace;

    if(token->Content == ".")
        return noSpace;

    if(lastToken->Content == ".")
        return noSpace;

    if(token->Content == "'s")
        return noSpace;
    
    if(token->Content == "[" || lastToken->Content == "[" || token->Content == "]")
        return noSpace;

    return space;
}

void PrintProgramToConsole(const Program* p)
{
    int lastIndentLevel = 0;
    for(auto& codeLine: p->Lines)
    {
        if(codeLine.Level < lastIndentLevel)
        {
            std::cout << std::endl;
        }

        for(int i=0; i<codeLine.Level; i++)
        {
            std::cout << IndentStr;
        }

        Token* lastToken = nullptr;
        Token* nextToken = nullptr;
        for(size_t i=0; i<codeLine.Tokens.size(); i++)
        {
            auto token = codeLine.Tokens[i];
            if(i+1 < codeLine.Tokens.size())
                nextToken = codeLine.Tokens[i+1];

            auto color = GetTokenColor(token, lastToken, nextToken);
            std::cout << color << IfNeededAddSpace(token, lastToken) << Formatted(token);
            lastToken = token;
        }

        std::cout << std::endl;

        lastIndentLevel = codeLine.Level;
    }
}