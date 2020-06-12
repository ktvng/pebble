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
#include "arch.h"
#include "token.h"
#include "object.h"
#include "diagnostics.h"


std::vector<Reference*> GlobalReferences;
std::stringstream ErrorBuffer;
static bool ErrorFlag = false;




String Message(String message, ...)
{
    va_list vl;
    va_start(vl, message);

    String expandedMessage = "";

    // expands message
    bool expandFlag = false;
    for(size_t i=0; i<message.size(); i++)
    {
        if(message.at(i) == '\\')
        {
            expandedMessage += message.at(i++);
            continue;
        }
        if(message.at(i) == '%')
        {
            expandFlag = true;
            continue;
        }
        if(expandFlag)
        {
            expandFlag = false;
            switch(message.at(i))
            {
                case 'i':
                expandedMessage += std::to_string(va_arg(vl, int));
                break;

                case 's':
                expandedMessage += va_arg(vl, String);
                break;

                case 'd':
                expandedMessage += std::to_string(va_arg(vl, double));
                break;

                default:
                break;
            }
            continue;
        }
        expandedMessage += message.at(i);
    }
    return expandedMessage;
}

void ReportError(String expandedMessage)
{
    ErrorBuffer << expandedMessage << std::endl;
    ErrorFlag = true;
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
    ReportError(Message("cannot add types %s and %s", lRef.ToObject->Class, rRef.ToObject->Class));
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
            ErrorPrint(op->LineNumber);
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
            return ltp1.Probability > ltp2.Probability;
        });
    lineType = typeProbabilities.at(0).Type;
}

void DecideOperands(const OperationType& lineType, const std::string& line, std::vector<Operation*>& operands, int lineNumber)
{
    std::istringstream iss(line);
    std::vector<std::string> tokens { std::istream_iterator<std::string>(iss), 
        std::istream_iterator<std::string>() };
    if(lineType == OperationType::Define)
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

String RemoveCommas(String line)
{
    String returnString = "";
    for(size_t i=0; i<line.size(); i++)
    {
        if(line.at(i) == ',')
            continue;
        returnString += line.at(i);
    }
    
    return returnString;
}

// commas allow line breaks TODO: WORK HERE
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

    std::string line = "test Of the Token 334 parser 3.1 haha \"this is awesome\" True";
    TokenList l = LexLine(line);
    PrintDiagnostics(l);

    return 0;
}