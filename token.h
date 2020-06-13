#ifndef __TOKEN_H
#define __TOKEN_H

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

#endif