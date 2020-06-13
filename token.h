#ifndef __TOKEN_H
#define __TOKEN_H

TokenList LexLine(const std::string& line);
std::string GetStringTokenType(TokenType type);
Token* FindToken(const TokenList& tokens, std::string str);
TokenList LeftOfToken(const TokenList& tokens, Token* token);
TokenList RightOfToken(const TokenList& tokens, Token* token);

#endif