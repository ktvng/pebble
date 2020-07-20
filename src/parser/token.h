#ifndef __TOKEN_H
#define __TOKEN_H

#include "abstract.h"

// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// used in tokenization to classify each token
enum class TokenType
{
    Simple,                 // any general word/symbol not matching another type
    Reference,              // encodes a Reference name; must start with capital letter
    Integer,                // a whole number
    String,                 // a line of characters enclosed by double quotes
    Decimal,                // a number with a decimal point
    Boolean,                // true of false in any case
};

/// a token contains information about chunks of parsed string including
/// [Type] describing the token's TokenType
/// [Content] being the string bit it refers to
/// [Position] what position in a line does the token occur
struct Token
{
    TokenType Type;
    String Content;
    int Position;
};

/// a list of TokenTypes which represent primitive objects
const std::vector<TokenType> PrimitiveTokenTypes {
    TokenType::Integer,
    TokenType::String,
    TokenType::Decimal,
    TokenType::Boolean,
};

/// tokens which represent any Object 
const std::vector<TokenType> ObjectTokenTypes {
    TokenType::Integer,
    TokenType::String,
    TokenType::Decimal,
    TokenType::Boolean,
    TokenType::Reference,
};

typedef std::vector<Token*> TokenList;


// ---------------------------------------------------------------------------------------------------------------------
// Methods

void DeleteTokenList(TokenList tokenList);

/// returns a new string with each character of [str] changed to lowercase
String ToLowerCase(const String& str);

// takes in a single line (no newlines) and returns a list of tokens representing the line
TokenList LexLine(const String& line);

// returns the string value for a category of the enum class TokenType
String GetStringTokenType(TokenType type);

// finds a token with token-Content equal to [str] in a list [tokens]
Token* FindToken(const TokenList& tokens, std::string str);

// returns a new TokenList containing all tokens to the left of [token], not including [token],
// and renumbered to begin at 0
TokenList LeftOfToken(const TokenList& tokens, Token* token);

// returns a new TokenList containing all tokens to the right of [token], not including [token],
// and renumbered to begin at 0
TokenList RightOfToken(const TokenList& tokens, Token* token);



// ---------------------------------------------------------------------------------------------------------------------
// Token Matching

/// finds the next token of [type] from [pos] in [tokens] and sets [pos] to the position after this token
/// returns nullptr and sets [pos] = -1 if no token exists
Token* NextTokenMatching(const TokenList& tokens, TokenType type, int& pos);

/// finds the next token of [type] from [pos] in [tokens] from the start
/// returns nullptr and sets [pos] = -1 if no token exists
Token* NextTokenMatching(const TokenList& tokens, TokenType type);

/// finds the next token of TokenType type in [types] from [pos] in [tokens] from the start
/// returns nullptr and sets [pos] = -1 if no token exists
Token* NextTokenMatching(const TokenList& tokens, std::vector<TokenType> types);

/// finds the next token of TokenType type in [types] from [pos] in [tokens] from [pos] and sets [pos] to the position
/// after this token. returns nullptr and sets [pos] = -1 if no token exists
Token* NextTokenMatching(const TokenList& tokens, std::vector<TokenType> types, int& pos);

/// finds the next token with token->Content matching in [content] from [pos] in [tokens] from [pos] and sets [pos] to the position
/// after this token. returns nullptr and sets [pos] = -1 if no token exists
Token* NextTokenMatching(const TokenList& tokens, std::vector<String> contents, int& pos);
Token* NextTokenMatching(const TokenList& tokens, std::vector<String> contents);
Token* NextTokenMatching(const TokenList& tokens, String content, int& pos);
Token* NextTokenMatching(const TokenList& tokens, String content);

/// returns true if [token] is one of the TokenTypes in [types]
bool TokenMatchesType(Token* token, TokenType type);
bool TokenMatchesType(Token* token, std::vector<TokenType> types);

/// returns true if [token] is has content matching one of the Strings in [contents]
bool TokenMatchesContent(Token* token, String content);
bool TokenMatchesContent(Token* token, std::vector<String> contents);

bool TokenListContainsContent(const TokenList& tokenList, std::vector<String> contents);

#endif
