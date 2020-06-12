#ifndef __MAIN_H
#define __MAIN_H

#include <string>
#include <vector>



struct Reference;
struct Object;
struct Block;
struct Operation;
struct Token;
struct LineTypeProbability;


typedef std::vector<Token> TokenList;
typedef std::string ObjectClass;
typedef std::string String;


const ObjectClass IntegerClass = "Integer";
const ObjectClass DecimalClass = "Decimal";
const ObjectClass StringClass = "String";
const ObjectClass BooleanClass = "Boolean";
const ObjectClass NullClass = "Null";

const std::string returnReferenceName = "returnObject";

const bool c_DEBUG = true;
const bool c_ERROR = true;


void PrintReference(const Reference& ref);
Operation* ParseLine(const std::string& line, int lineNumber);
std::string GetStringValue(const Object& obj);
TokenList LexLine(const std::string& line);

#endif