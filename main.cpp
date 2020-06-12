#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <functional>
#include <random>
#include <algorithm>
#include <numeric>
#include <tuple>
#include <cstdarg>
#include <map>
#include <iterator>
#include <sstream>
#include <cctype>

#include "main.h"


// Structs
enum class ControlType
{
    If,
    While,
    Statement
};

enum class OperationType
{
    Define,
    Assign, //
    IsEqual,
    LessThan,
    GreaterThan,
    Add, //
    Subtract,
    Multiply,
    Divide,
    And, //
    Or,
    Not,
    Evaluate,
    Print,
    Return,
};

struct Reference
{
    std::string Name;
    Object* ToObject;
};

struct Object
{
    ObjectClass Class;
    std::vector<Reference> Attributes;
    void* Value;
};

struct Operation
{
    OperationType Type;
    std::vector<Operation*> Operands;
    Reference* Value;
    int LineNumber;
};

struct Block
{
    std::vector<Operation*> Operations;
};





void DebugPrint(std::string value)
{
    if(c_DEBUG)
        std::cout << value << "\n";
}

void ReportError(int lineNumber)
{
    if(c_ERROR)
    {
        std::string line;
        while(std::getline(ErrorBuffer, line))
        {
            std::cerr << "(!) Exception at line[" << lineNumber << "]: "  << line << "\n";
        }
    }
    ErrorFlag = false;
}







void Print(const Object& obj)
{
    std::cout << "| Class: " << obj.Class << "\n| Value: " << GetStringValue(obj)  << "\n"; 
}

void Print(const Reference& ref)
{
    std::cout << "| Name: " << ref.Name << "\n";
    Print(*ref.ToObject);
    std::cout << "\n";
}

void Print(const Operation& op, int level=0)
{
    std::string type;
    switch(op.Type){
        case OperationType::Add:
        type = "Add";
        break;

        case OperationType::Return:
        type = "Return";
        break;

        case OperationType::Print:
        type = "Print";
        break;

        case OperationType::Assign:
        type = "Assign";
        break;

        case OperationType::Define:
        type = "Define";
        break;

        default:
        type = "unimplemented";
        break;
    }
    std::cout << "OP---" << level << "\nType " << type << "\n";
    if(op.Type == OperationType::Return)
    {
        Print(*op.Value);
    }
    for(Operation* operand: op.Operands)
    {
        Print(*operand, level+1);
    }
}








Reference* MakeGeneric(std::string name, ObjectClass objClass)
{
    Reference* ref = new Reference;
    Object* obj = new Object;

    ref->Name = name;
    ref->ToObject = obj;

    obj->Class = objClass;
    
    return ref;
}

Reference* Make(std::string name, ObjectClass objClass, int value)
{
    Reference* ref = MakeGeneric(name, objClass);
    int* i = new int;
    *i = value;
    ref->ToObject->Value = i;

    return ref;
}

Reference* Make(std::string name, ObjectClass objClass, double value)
{
    Reference* ref = MakeGeneric(name, objClass);
    double* d = new double;
    *d = value;
    ref->ToObject->Value = d;

    return ref;
}

Reference* Make(std::string name, ObjectClass objClass, bool value)
{
    Reference* ref = MakeGeneric(name, objClass);
    bool* b = new bool;
    *b = value;
    ref->ToObject->Value = b;

    return ref;
}

Reference* Make(std::string name, ObjectClass objClass, std::string value)
{
    Reference* ref = MakeGeneric(name, objClass);
    std::string* s = new std::string;
    *s = value;
    ref->ToObject->Value = s;
    
    return ref;
}

Reference* Make(std::string name)
{
    static Object nullObject;
    nullObject.Class = NullClass;
    
    Reference* ref = new Reference { name, &nullObject };
    return ref;
}




std::string GetStringValue(const Object& obj)
{
    if(obj.Class == IntegerClass)
    {
        return std::to_string(*static_cast<int*>(obj.Value));
    }
    else if(obj.Class == DecimalClass)
    {
        return std::to_string(*static_cast<double*>(obj.Value));
    }
    else if(obj.Class == BooleanClass)
    {
        if(*static_cast<bool*>(obj.Value))
        {
            return "true";
        }
        return "false";
    }
    else if(obj.Class == StringClass)
    {
        return *static_cast<std::string*>(obj.Value);
    }
    else if(obj.Class == NullClass)
    {
        return "null";
    }
    DebugPrint("unknown class");
    return "";
}



///
void Assign(Reference& lRef, Reference& rRef)
{
    lRef.ToObject = rRef.ToObject;
}

/// 
void Print(Reference& ref)
{
    if(ref.ToObject->Class == IntegerClass ||
        ref.ToObject->Class == DecimalClass ||
        ref.ToObject->Class == StringClass ||
        ref.ToObject->Class == BooleanClass ||
        ref.ToObject->Class == NullClass)
    {
        std::cout << GetStringValue(*ref.ToObject) << "\n";
    }
}

///
bool IsNumeric(const Reference& ref)
{
    return ref.ToObject->Class == IntegerClass || ref.ToObject->Class == DecimalClass;
}

ObjectClass GetPrecedenceClass(const Object& obj1, const Object& obj2)
{
    if(obj1.Class == DecimalClass || obj2.Class == DecimalClass)
        return DecimalClass;
    return IntegerClass;
}

int GetIntValue(const Object& obj)
{
    if(obj.Class != IntegerClass)
    {
        DebugPrint("Object has no integer value");
        return 0;
    }
    return *static_cast<int*>(obj.Value);
}

double GetDecimalValue(const Object& obj)
{
    if(obj.Class == DecimalClass)
    {
        return *static_cast<double*>(obj.Value);
    }
    if(obj.Class == IntegerClass)
    {
        return static_cast<double>(*static_cast<int*>(obj.Value));
    }
    DebugPrint("Object has no decimal value");
    return 0;
}

bool GetBoolValue(const Object& obj)
{
    if(obj.Class == BooleanClass)
    {
        return *static_cast<bool*>(obj.Value);
    }
    else if (obj.Class == IntegerClass)
    {
        return *static_cast<int*>(obj.Value);
    }
    else
    {
        return true;
    }
}

///
Reference* Add(const Reference& lRef, const Reference& rRef)
{
    Reference* resultRef;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*(lRef.ToObject), *(rRef.ToObject));
        if(type == IntegerClass)
        {
            int value = GetIntValue(*lRef.ToObject) + GetIntValue(*rRef.ToObject);
            resultRef = Make(returnReferenceName, IntegerClass, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*(lRef.ToObject)) + GetDecimalValue(*(rRef.ToObject));
            resultRef = Make(returnReferenceName, DecimalClass, value);
        }
        else
        {
            resultRef = Make(returnReferenceName);
        }
        return resultRef;
    }

    resultRef = Make(returnReferenceName);
    ErrorFlag = true;
    ErrorBuffer << "cannot add types " << lRef.ToObject->Class << " and " << rRef.ToObject->Class + "\n";
    return resultRef;
}

///
Reference* And(const Reference& lRef, const Reference& rRef)
{
    bool b = GetBoolValue(*lRef.ToObject) && GetBoolValue(*rRef.ToObject);
    return Make(returnReferenceName, BooleanClass, b);
}



Reference* DoOperationOnReferences(Operation* op, std::vector<Reference*> operands)
{
    Reference* nullRef = Make(returnReferenceName);
    switch(op->Type)
    {
        case OperationType::Add:
        return Add(*operands.at(0), *operands.at(1));

        case OperationType::Print:
        Print(*operands.at(0));
        return nullRef;

        case OperationType::Assign:
        Assign(*operands.at(0), *operands.at(1));
        return nullRef;

        default:
        break;
    }
    return nullRef;
}

Reference* DoOperation(Operation* op)
{
    if(op->Type == OperationType::Return)
        return op->Value;
    else if (op->Type == OperationType::Define)
    {
        return Make(returnReferenceName);
    }
    else
    {
        std::vector<Reference*> operandReferences;
        for(Operation* operand: op->Operands)
        {
            Reference* operandRef = DoOperation(operand);
            operandReferences.push_back(operandRef);
        }
        return DoOperationOnReferences(op, operandReferences);
    }
    
}

void DoBlock(Block& codeBlock)
{
    for(Operation* op: codeBlock.Operations)
    {
        // PrintOperation(*op);
        // Reference* result = 
        DoOperation(op);
        if(ErrorFlag)
        {
            ReportError(op->LineNumber);
        }
    }
}






Operation* CreateReturnOperation(Reference* ref, int lineNumber)
{
    Operation* op = new Operation;
    op->Type = OperationType::Return;
    op->Value = ref;
    op->LineNumber = lineNumber;

    return op;
}



Reference* CreateReference(std::string name, std::string type, std::string value)
{
    if(type == IntegerClass)
    {
        return Make(name, IntegerClass, std::stoi(value));
    }
    else if(type == DecimalClass)
    {
        return Make(name, DecimalClass, std::stod(value));
    }
    else if(type == BooleanClass)
    {
        bool b = value == "true" ? true : false;
        return Make(name, BooleanClass, b);
    }
    else if(type == StringClass)
    {
        return Make(name, StringClass, value);
    }

    DebugPrint("Cannot create reference");
    return Make(name);
}

Reference* DecideReference(std::string name)
{
    for(Reference* ref: GlobalReferences)
    {
        if(ref->Name == name){
            return ref;
        }
    }
    DebugPrint("Cannot decide reference");
    return nullptr;
}


struct LineTypeProbability
{
    OperationType Type;
    double Probabilitiy;
};

void DecideLineTypeProbabilities(std::vector<LineTypeProbability>& typeProbabilities, const std::string line)
{
    size_t pos;
    if(pos = line.rfind("add"); pos != std::string::npos)
    {
        LineTypeProbability addType = { OperationType::Add, 10.0/pos };
        typeProbabilities.push_back(addType);
    }
    if(pos = line.rfind("define"); pos != std::string::npos)
    {
        LineTypeProbability defineType = { OperationType::Define, 10.0/pos };
        typeProbabilities.push_back(defineType);
    }
    if(pos = line.rfind("print"); pos != std::string::npos)
    {
        LineTypeProbability printType = { OperationType::Print, 10.0/pos };
        typeProbabilities.push_back(printType);
    }
    if(pos = line.rfind("="); pos != std::string::npos)
    {
        LineTypeProbability assignType = { OperationType::Assign, 10.0/pos };
        typeProbabilities.push_back(assignType);
    }
    LineTypeProbability returnType = { OperationType::Return, 1 };
    typeProbabilities.push_back(returnType);
}

void DecideLineType(std::vector<LineTypeProbability>& typeProbabilities, OperationType& lineType)
{
    std::sort(typeProbabilities.begin(), typeProbabilities.end(),
        [](const LineTypeProbability& ltp1, const LineTypeProbability& ltp2){
            return ltp1.Probabilitiy > ltp2.Probabilitiy;
        });
    lineType = typeProbabilities.at(0).Type;
}

void DecideOperands(const OperationType& lineType, const std::string& line, std::vector<Operation*>& operands, int lineNumber)
{
    std::istringstream iss(line);
    std::vector<std::string> tokens { std::istream_iterator<std::string>(iss), 
        std::istream_iterator<std::string>() };
    if(lineType == OperationType:: Define)
    {
        std::string type = tokens.at(1);
        std::string name = tokens.at(2);
        std::string value = tokens.at(3);

        Reference* r = CreateReference(name, type, value);

        GlobalReferences.push_back(r);
        return;
    }
    else if(lineType == OperationType::Add)
    {
        Reference* arg1 = DecideReference(tokens.at(1));
        Reference* arg2 = DecideReference(tokens.at(2));

        Operation* op1 = CreateReturnOperation(arg1, lineNumber);
        Operation* op2 = CreateReturnOperation(arg2, lineNumber);

        operands.push_back(op1);
        operands.push_back(op2);
    }
    else if(lineType == OperationType::Print)
    {
        Reference* arg1 = DecideReference(tokens.at(1));
        
        Operation* op1 = CreateReturnOperation(arg1, lineNumber);
        operands.push_back(op1);
    }
    else if(lineType == OperationType::Assign)
    {
        Reference* arg1 = DecideReference(tokens.at(0));
        Operation* op1 = CreateReturnOperation(arg1, lineNumber);

        int pos = line.rfind("=");
        Operation* op2 = ParseLine(line.substr(pos+1), lineNumber);
        op2->LineNumber = lineNumber;

        operands.push_back(op1);
        operands.push_back(op2);
        
    }
    else if(lineType == OperationType::Return)
    {
        Reference* arg1 = DecideReference(tokens.at(0));
        Operation* op1 = CreateReturnOperation(arg1, lineNumber);

        operands.push_back(op1);
    }
}


char LastNonWhitespaceChar(std::string& line)
{
    int i;
    for(i=line.size()-1; i>=0 && line.at(i) != ' '; i--);
    return i; 
}

std::string RemoveCommas(std::string line)
{
    std::string returnString = "";
    for(size_t i=0; i<line.size(); i++)
    {
        if(line.at(i) == ',')
            continue;
        returnString += line.at(i);
    }
    
    return returnString;
}

// commas allow line breaks
std::string GetEffectiveLine(std::fstream& file, int& lineNumber)
{
    std::string fullLine = "";
    std::string newLine;
    do
    {
        lineNumber++;
        if(!std::getline(file, newLine))
            break;
        fullLine += newLine;

    } while (LastNonWhitespaceChar(newLine) == ',' || newLine.size() == 0);
    return RemoveCommas(fullLine);
}

Operation* ParseLine(const std::string& line, int lineNumber)
{
    std::vector<LineTypeProbability> typeProbabilities;
    DecideLineTypeProbabilities(typeProbabilities, line);

    OperationType lineType;
    DecideLineType(typeProbabilities, lineType);

    std::vector<Operation*> operands;
    DecideOperands(lineType, line, operands, lineNumber);

    Operation* op = new Operation;
    op->Type = lineType;
    op->Operands = operands;
    op->LineNumber = lineNumber;
    if(lineType == OperationType::Return)
    {
        op->Value = operands.at(0)->Value;
    }

    return op;
}

Block Parse(const std::string filepath, int& lineNumber){
    Block parsedBlock;
    std::fstream file;
    file.open(filepath, std::ios::in);

    int lineEndPos = lineNumber;
    for(std::string line = GetEffectiveLine(file, lineEndPos); line != ""; line = GetEffectiveLine(file, lineEndPos))
    {
        Operation* op = ParseLine(line, lineNumber);
        parsedBlock.Operations.push_back(op);

        lineNumber = lineEndPos;
    }

    return parsedBlock;
}




enum class TokenType
{
    Simple,
    Reference,
    Integer,
    String,
    Decimal,
    Boolean,
};

struct Token
{
    TokenType Type;
    std::string Content;
};

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
        for(++position; position < line.size() && line.at(position) != '"'; position++)
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

void PrintTokenList(TokenList tokenList)
{
    std::cout << "TOKENS---\n";
    for(Token t: tokenList)
    {
        std::cout << "| Type: " << GetStringTokenType(t.Type) << "\t Content: " << t.Content << "\n";
    }
}



int main()
{
    int lineNumber = 1;
    Block b = Parse(".\\program", lineNumber);

    // PRINT OPERATIONS
    // for(Operation* op: b.Operations)
    // {
    //     PrintOperation(*op);
    // }

    std::cout << "####################\n";

    DoBlock(b);

    // PRINT GLOBALREFRENCES
    // for(auto elem: GlobalReferences)
    // {
    //     std::cout << &elem << "\n";
    //     PrintReference(*elem);
    // }

    // std::string line = "test Of the Token 334 parser 3.1 haha \"this is awesome\" True";
    // TokenList l = LexLine(line);
    // PrintTokenList(l);

    return 0;
}