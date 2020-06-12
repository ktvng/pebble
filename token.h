#ifndef __TOKEN_H
#define __TOKEN_H

TokenList LexLine(const std::string& line);
std::string GetStringTokenType(TokenType type);

#endif