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

// Error reporting
std::stringstream ErrorBuffer;
static bool ErrorFlag = false;

// adds an error to the error buffer
void ReportError(String expandedMessage)
{
    ErrorBuffer << expandedMessage << std::endl;
    ErrorFlag = true;
}



// Atomic Operations
///
Reference* Assign(Reference& lRef, Reference& rRef)
{
    lRef.ToObject = rRef.ToObject;
    return CreateReference(returnReferenceName, lRef.ToObject);
}

/// 
Reference* Print(Reference& ref)
{
    if(ref.ToObject->Class == IntegerClass ||
        ref.ToObject->Class == DecimalClass ||
        ref.ToObject->Class == StringClass ||
        ref.ToObject->Class == BooleanClass ||
        ref.ToObject->Class == NullClass)
    {
        std::cout << GetStringValue(*ref.ToObject) << "\n";
    }

    return CreateReference(returnReferenceName, ref.ToObject);
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
            resultRef = CreateReference(returnReferenceName, IntegerClass, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*(lRef.ToObject)) + GetDecimalValue(*(rRef.ToObject));
            resultRef = CreateReference(returnReferenceName, DecimalClass, value);
        }
        else
        {
            resultRef = CreateReference(returnReferenceName);
        }
        return resultRef;
    }

    resultRef = CreateReference(returnReferenceName);
    ReportError(Message("cannot add types %s and %s", lRef.ToObject->Class, rRef.ToObject->Class));
    return resultRef;
}

///
Reference* And(const Reference& lRef, const Reference& rRef)
{
    bool b = GetBoolValue(*lRef.ToObject) && GetBoolValue(*rRef.ToObject);
    return CreateReference(returnReferenceName, BooleanClass, b);
}





// Program execution

// update when adding operations
Reference* DoOperationOnReferences(Operation* op, std::vector<Reference*> operands)
{
    Reference* nullRef = CreateReference(returnReferenceName);
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
        return CreateReference(returnReferenceName);
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
            RuntimeErrorPrint(op->LineNumber);
        }
    }
}



// Creating operations
Operation* CreateReturnOperation(Reference* ref)
{
    Operation* op = new Operation;
    op->Type = OperationType::Return;
    op->Value = ref;
    op->LineNumber = 0;
    op->Operands = std::vector<Operation*>();

    return op;
}




// Decide Probabilities
void DecideProbabilityAdd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(Token* pos = FindToken(tokens, "add"); pos != nullptr)
    {
        OperationTypeProbability addType = { OperationType::Add, 10.0/pos->Position };
        typeProbabilities.push_back(addType);
    }
    
}

void DecideProbabilityDefine(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(Token* pos = FindToken(tokens, "define"); pos != nullptr)
    {
        OperationTypeProbability defineType = { OperationType::Define, 10.0/pos->Position };
        typeProbabilities.push_back(defineType);
    }
}

void DecideProbabilityPrint(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(Token* pos = FindToken(tokens, "print"); pos != nullptr)
    {
        OperationTypeProbability printType = { OperationType::Print, 10.0/pos->Position };
        typeProbabilities.push_back(printType);
    }
}

void DecideProbabilityAssign(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(Token* pos = FindToken(tokens, "="); pos != nullptr)
    {
        OperationTypeProbability assignType = { OperationType::Assign, 10.0/pos->Position };
        typeProbabilities.push_back(assignType);
    }
}

void DecideProbabilityReturn(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    OperationTypeProbability returnType = { OperationType::Return, 1 };
    typeProbabilities.push_back(returnType);
}

void DecideProbabilityIsEqual(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{

}

void DecideProbabilityIsLessThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityIsGreaterThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilitySubtract(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityMultiply(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityDivide(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityAnd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityOr(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityNot(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityEvaluate(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}






// Decide Operands
// should edit token list remove used tokens
void DecideOperandsAdd(TokenList& tokens, std::vector<Operation*>& operands)
{
    Reference* arg1 = DecideReference(tokens.at(1)->Content);
    Reference* arg2 = DecideReference(tokens.at(2)->Content);

    Operation* op1 = CreateReturnOperation(arg1);
    Operation* op2 = CreateReturnOperation(arg2);

    operands.push_back(op1);
    operands.push_back(op2);
}

void DecideOperandsDefine(TokenList& tokens, std::vector<Operation*>& operands)
{
    Token* name = tokens.at(1);
    Token* value = tokens.at(2);

    Reference* r = CreateReference(name, value);

    GlobalReferences.push_back(r);
    return;
}

void DecideOperandsPrint(TokenList& tokens, std::vector<Operation*>& operands)
{
    Reference* arg1 = DecideReference(tokens.at(1)->Content);
    
    Operation* op1 = CreateReturnOperation(arg1);
    operands.push_back(op1);
}

void DecideOperandsAssign(TokenList& tokens, std::vector<Operation*>& operands)
{
    Reference* arg1 = DecideReference(tokens.at(0)->Content);
    Operation* op1 = CreateReturnOperation(arg1);

    TokenList leftTokens = RightOfToken(tokens, tokens.at(1));
    Operation* op2 = ParseLine(leftTokens);

    operands.push_back(op1);
    operands.push_back(op2);
}

void DecideOperandsIsEqual(TokenList& tokens, std::vector<Operation*>& operands)
{

}

void DecideOperandsIsLessThan(TokenList& tokens, std::vector<Operation*>& operands)
{
    
}

void DecideOperandsIsGreaterThan(TokenList& tokens, std::vector<Operation*>& operands)
{
    
}

void DecideOperandsSubtract(TokenList& tokens, std::vector<Operation*>& operands)
{
    
}

void DecideOperandsMultiply(TokenList& tokens, std::vector<Operation*>& operands)
{
    
}

void DecideOperandsDivide(TokenList& tokens, std::vector<Operation*>& operands)
{
    
}

void DecideOperandsAnd(TokenList& tokens, std::vector<Operation*>& operands)
{
    
}

void DecideOperandsOr(TokenList& tokens, std::vector<Operation*>& operands)
{
    
}

void DecideOperandsNot(TokenList& tokens, std::vector<Operation*>& operands)
{
    
}

void DecideOperandsEvaluate(TokenList& tokens, std::vector<Operation*>& operands)
{
    
}

void DecideOperandsReturn(TokenList& tokens, std::vector<Operation*>& operands)
{
    Reference* arg1;

    if(tokens.at(0)->Type == TokenType::Reference)
        arg1 = DecideReference(tokens.at(0)->Content);
    else if(tokens.at(0)->Type != TokenType::Simple){
        arg1 = CreateReference(returnReferenceName, tokens.at(0));
    }

    Operation* op1 = CreateReturnOperation(arg1);

    operands.push_back(op1);
}






// Meta Parsing + Deciding
Reference* DecideReference(std::string name)
{
    for(Reference* ref: GlobalReferences)
    {
        if(ref->Name == name){
            return ref;
        }
    }
    DebugPrint("Cannot decide reference");
    return CreateReference(returnReferenceName);
}

void DecideLineTypeProbabilities(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    DecideProbabilityDefine(typeProbabilities, tokens);
    DecideProbabilityAssign(typeProbabilities, tokens);
    DecideProbabilityIsEqual(typeProbabilities, tokens);
    DecideProbabilityIsLessThan(typeProbabilities, tokens);
    DecideProbabilityIsGreaterThan(typeProbabilities, tokens);
    DecideProbabilityAdd(typeProbabilities, tokens);
    DecideProbabilitySubtract(typeProbabilities, tokens);
    DecideProbabilityMultiply(typeProbabilities, tokens);
    DecideProbabilityDivide(typeProbabilities, tokens);
    DecideProbabilityAnd(typeProbabilities, tokens);
    DecideProbabilityOr(typeProbabilities, tokens);
    DecideProbabilityNot(typeProbabilities, tokens);
    DecideProbabilityEvaluate(typeProbabilities, tokens);
    DecideProbabilityPrint(typeProbabilities, tokens);
    DecideProbabilityReturn(typeProbabilities, tokens);
}

// TODO: figure out how to decide line type
void DecideLineType(PossibleOperationsList& typeProbabilities, const TokenList& tokens, LineType& lineType)
{
    lineType = LineType::Atomic;
}

void DecideOperationType(PossibleOperationsList& typeProbabilities, OperationType& lineType)
{
    std::sort(typeProbabilities.begin(), typeProbabilities.end(),
        [](const OperationTypeProbability& ltp1, const OperationTypeProbability& ltp2){
            return ltp1.Probability > ltp2.Probability;
        });
    lineType = typeProbabilities.at(0).Type;
}

void DecideOperands(const OperationType& lineType, TokenList& tokens, std::vector<Operation*>& operands)
{
    switch(lineType)
    {
        case OperationType::Define:
        DecideOperandsDefine(tokens, operands);
        break;

        case OperationType::Assign:
        DecideOperandsAssign(tokens, operands);
        break;

        case OperationType::IsEqual:
        DecideOperandsIsEqual(tokens, operands);
        break;

        case OperationType::IsLessThan:
        DecideOperandsIsLessThan(tokens, operands);
        break;

        case OperationType::IsGreaterThan:
        DecideOperandsIsGreaterThan(tokens, operands);
        break;

        case OperationType::Add:
        DecideOperandsAdd(tokens, operands);
        break;

        case OperationType::Subtract:
        DecideOperandsSubtract(tokens, operands);
        break;

        case OperationType::Multiply:
        DecideOperandsMultiply(tokens, operands);
        break;

        case OperationType::Divide:
        DecideOperandsDivide(tokens, operands);
        break;

        case OperationType::And:
        DecideOperandsAnd(tokens, operands);
        break;

        case OperationType::Or:
        DecideOperandsOr(tokens, operands);
        break;

        case OperationType::Not:
        DecideOperandsNot(tokens, operands);
        break;

        case OperationType::Evaluate:
        DecideOperandsEvaluate(tokens, operands);
        break;

        case OperationType::Print:
        DecideOperandsPrint(tokens, operands);
        break;

        case OperationType::Return:
        DecideOperandsReturn(tokens, operands);
        break;
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

// commas allow line breaks TODO: WORK HERE
std::string GetEffectiveLine(std::fstream& file, int& lineNumber, int& lineStart)
{
    std::string fullLine = "";
    std::string newLine;

    do
    {
        lineNumber++;
        if(!std::getline(file, newLine))
            break;
        if(newLine != "")
            lineStart = lineNumber - 1;
        std::cout << "(" << newLine << ")";
        fullLine += newLine;

    } while (LastNonWhitespaceChar(newLine) == ',' || newLine.size() == 0);
    return RemoveCommas(fullLine);
}





Operation* ParseOutAtomic(PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    OperationType opType;
    DecideOperationType(typeProbabilities, opType);

    std::vector<Operation*> operands;
    DecideOperands(opType, tokens, operands);

    Operation* op = new Operation;
    op->Type = opType;
    op->Operands = operands;
    if(opType == OperationType::Return)
    {
        op->Operands = std::vector<Operation*>();
        op->Value = operands.at(0)->Value;
    }

    return op;
}

// TODO: Implement
Operation* ParseComposite(PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    return nullptr;
}



void NumberOperation(Operation* op, int lineNumber)
{
    op->LineNumber = lineNumber;
    for(Operation* operand: op->Operands)
    {
        NumberOperation(operand, lineNumber);
    }
}

Operation* ParseLine(TokenList& tokens)
{
    PossibleOperationsList typeProbabilities;
    DecideLineTypeProbabilities(typeProbabilities, tokens);

    LineType lineType;
    DecideLineType(typeProbabilities, tokens, lineType);

    switch(lineType)
    {
        case LineType::Atomic:
        return ParseOutAtomic(typeProbabilities, tokens);

        case LineType::Composite:
        return ParseComposite(typeProbabilities, tokens);

        case LineType::If:

        case LineType::While:

        default:
        DebugPrint("ParseLine is unimplemented for this case");
        return nullptr;
    }
}

Block ParseBlock(const std::string filepath, int& lineNumber){
    Block parsedBlock;
    std::fstream file;
    file.open(filepath, std::ios::in);

    int lineEndPos = lineNumber;
    int lineStart = lineNumber;
    for(std::string line = GetEffectiveLine(file, lineEndPos, lineStart); line != ""; line = GetEffectiveLine(file, lineEndPos, lineStart))
    {
        TokenList tokens = LexLine(line);
        Operation* op = ParseLine(tokens);

        NumberOperation(op, lineStart);
        parsedBlock.Operations.push_back(op);

        lineNumber = lineEndPos;
        std::cout << lineNumber;
    }

    return parsedBlock;
}








int main()
{
    bool PRINT_OPERATIONS = false;

    int lineNumber = 1;
    Block b = ParseBlock(".\\program", lineNumber);

    // PRINT OPERATIONS
    if(PRINT_OPERATIONS)
    {
    for(Operation* op: b.Operations)
        {
            PrintDiagnostics(*op);
        }
    }


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
    // PrintDiagnostics(l);

    return 0;
}