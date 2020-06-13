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

/// flag which is true whenever the ErrorBuffer contains a pending error message
static bool ErrorFlag = false;

/// adds an error to the error buffer which will be printed when RuntimeErrorPrint is called
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
    return CreateReference(c_returnReferenceName, lRef.ToObject);
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

    return CreateReference(c_returnReferenceName, ref.ToObject);
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
            resultRef = CreateReferenceToNewObject(c_returnReferenceName, IntegerClass, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*(lRef.ToObject)) + GetDecimalValue(*(rRef.ToObject));
            resultRef = CreateReferenceToNewObject(c_returnReferenceName, DecimalClass, value);
        }
        else
        {
            resultRef = CreateNullReference();
        }
        return resultRef;
    }

    resultRef = CreateNullReference();
    ReportError(Message("cannot add types %s and %s", lRef.ToObject->Class, rRef.ToObject->Class));
    return resultRef;
}

///
Reference* And(const Reference& lRef, const Reference& rRef)
{
    bool b = GetBoolValue(*lRef.ToObject) && GetBoolValue(*rRef.ToObject);
    return CreateReferenceToNewObject(c_returnReferenceName, BooleanClass, b);
}





// Program execution

/// executes an operation [op] on the ordered list of reference [operands]. should only be called
/// through DoOperation
Reference* DoOperationOnReferences(Operation* op, std::vector<Reference*> operands)
{
    switch(op->Type)
    {
        case OperationType::Add:
        return Add(*operands.at(0), *operands.at(1));

        case OperationType::Print:
        return Print(*operands.at(0));

        case OperationType::Assign:
        return Assign(*operands.at(0), *operands.at(1));

        default:
        DebugPrint("(DoOperationOnReference) unknown/unimplemented OperationType");
        return CreateNullReference();
    }
}

/// executes an operation [op]
Reference* DoOperation(Operation* op)
{
    if(op->Type == OperationType::Return)
        return op->Value;
    else if (op->Type == OperationType::Define)
    {
        return CreateNullReference();
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

/// executes a [codeBlock]
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

/// creates a Operation with OperationType::Return to return [ref]
Operation* CreateReturnOperation(Reference* ref)
{
    Operation* op = new Operation;
    op->Type = OperationType::Return;
    op->Value = ref;

    return op;
}

/// adds a new return Operation for [ref] to [operands]
void AddReferenceReturnOperationTo(OperationsList& operands, Reference* ref)
{
    operands.push_back(CreateReturnOperation(ref));
}

/// adds an existing Operation [op] to [operands]
void AddOperationTo(OperationsList& operands, Operation* op)
{
    operands.push_back(op);
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
void DecideOperandsAdd(TokenList& tokens, OperationsList& operands) // EDIT
{
    int pos = 0;
 
    Reference* arg1 = DecideReferenceOf( NextTokenMatching(tokens, ObjectTokenTypes, pos) );
    Reference* arg2 = DecideReferenceOf( NextTokenMatching(tokens, ObjectTokenTypes, pos) );

    AddReferenceReturnOperationTo(operands, arg1);
    AddReferenceReturnOperationTo(operands, arg2);
}

void DecideOperandsDefine(TokenList& tokens, OperationsList& operands)
{
    Token* name = NextTokenMatching(tokens, TokenType::Reference);
    Token* value = NextTokenMatching(tokens, PrimitiveTokenTypes);

    if(name == nullptr)
        DebugPrint("Unknown reference");
    if(value == nullptr)
        DebugPrint("Unknown value");

    Reference* r = CreateReferenceToNewObject(name, value);


    GlobalReferences.push_back(r);
}

void DecideOperandsPrint(TokenList& tokens, OperationsList& operands)
{
    Reference* arg1 = DecideReferenceOf(NextTokenMatching(tokens, ObjectTokenTypes));
    AddReferenceReturnOperationTo(operands, arg1);
}

void DecideOperandsAssign(TokenList& tokens, OperationsList& operands)
{
    int pos = 0;
    Reference* arg1 = DecideReferenceOf(NextTokenMatching(tokens, TokenType::Reference, pos));

    TokenList rightTokens = RightOfToken(tokens, tokens.at(pos));
    Operation* op2 = ParseLine(rightTokens);

    AddReferenceReturnOperationTo(operands, arg1);
    AddOperationTo(operands, op2);
}

void DecideOperandsIsEqual(TokenList& tokens, OperationsList& operands)
{

}

void DecideOperandsIsLessThan(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsIsGreaterThan(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsSubtract(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsMultiply(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsDivide(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsAnd(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsOr(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsNot(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsEvaluate(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsReturn(TokenList& tokens, OperationsList& operands)
{
    Reference* arg1 = DecideReferenceOf(NextTokenMatching(tokens, ObjectTokenTypes));
    AddReferenceReturnOperationTo(operands, arg1);
}






// Meta Parsing + Deciding
/// returns true if [token] corresponds to [ref]
bool TokenMatchesReference(Token* token, Reference* ref) // MAJOR
{
    return token->Content == ref->Name;
}

/// returns a pointer to a Reference if [token] corresponds to an existing reference and nullptr if 
/// no matching token can be resolved;
Reference* DecideExistingReferenceFor(Token* token) // MAJOR: Add scope
{
    if(TokenMatchesType(token, TokenType::Reference))
    {
        for(Reference* ref: GlobalReferences)
        {
            if(TokenMatchesReference(token, ref))
                return ref;
        }
        DebugPrint("cannot resolve reference for token");
    }
    return nullptr;
}

/// returns a new primitive reference if [token] is a PrimitiveTokenType, otherwise nullptr
Reference* DecideNewReferenceFor(Token* token)
{
    if(TokenMatchesType(token, PrimitiveTokenTypes))
    {
        return CreateReferenceToNewObject(c_returnReferenceName, token);
    }
    return nullptr;
}

/// given a Token* [token], will return either an existing reference (or null reference if none match)
/// or a new reference to a primitive object
Reference* DecideReferenceOf(Token* token)
{
    if(token == nullptr)
        return CreateNullReference();

    Reference* tokenRef;

    tokenRef = DecideExistingReferenceFor(token);
    if(tokenRef != nullptr)
        return tokenRef;

    tokenRef = DecideNewReferenceFor(token);
    if(tokenRef != nullptr)
        return tokenRef;

    return CreateNullReference();
}




/// decide the probability that a line represented by [tokens] corresponds to each of the atomic operations and stores
/// this in [typeProbabilities]
void DecideOperationTypeProbabilities(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
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
/// assign [lineType] based on [typeProbabitlites] for each atomic operation
void DecideLineType(PossibleOperationsList& typeProbabilities, const TokenList& tokens, LineType& lineType) // MAJOR
{
    lineType = LineType::Atomic;
}

/// given pre-completed [typeProbabilities], decides what operation is most likey
void DecideOperationType(PossibleOperationsList& typeProbabilities, OperationType& opType) // MAJOR
{
    std::sort(typeProbabilities.begin(), typeProbabilities.end(),
        [](const OperationTypeProbability& ltp1, const OperationTypeProbability& ltp2){
            return ltp1.Probability > ltp2.Probability;
        });
    opType = typeProbabilities.at(0).Type;
}

/// decides and adds the operations for the Operation of [opType] to [operands] 
void DecideOperands(const OperationType& opType, TokenList& tokens, OperationsList& operands)
{
    switch(opType)
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
        fullLine += newLine;

    } while (LastNonWhitespaceChar(newLine) == ',' || newLine.size() == 0);
    return RemoveCommas(fullLine);
}




/// parses an atomic operation into an Operation tree
Operation* ParseOutAtomic(PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    OperationType opType;
    DecideOperationType(typeProbabilities, opType);

    OperationsList operands;
    DecideOperands(opType, tokens, operands);

    Operation* op = new Operation;
    op->Type = opType;
    op->Operands = operands;
    if(opType == OperationType::Return)
    {
        op->Operands = OperationsList();
        op->Value = operands.at(0)->Value;
    }

    return op;
}

// TODO: Implement
/// parses a composite operation into an Operation tree
Operation* ParseComposite(PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    return nullptr;
}


/// assigns [lineNumber] to be the LineNumber for each operation in the Operation tree of [op]
void NumberOperation(Operation* op, int lineNumber)
{
    op->LineNumber = lineNumber;
    for(Operation* operand: op->Operands)
    {
        NumberOperation(operand, lineNumber);
    }
}




/// parses a line of code
Operation* ParseLine(TokenList& tokens)
{
    PossibleOperationsList typeProbabilities;
    DecideOperationTypeProbabilities(typeProbabilities, tokens);

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

/// parses a block of code
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
    }

    return parsedBlock;
}








int main()
{
    bool PRINT_OPERATIONS = false;
    bool PRINT_GLOBAL_REFS = false;
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

    std::cout << "####################\n";
    std::cout << "FINISH";
    
    // PRINT GLOBALREFRENCES
    if(PRINT_GLOBAL_REFS)
    {
        for(auto elem: GlobalReferences)
        {
            std::cout << &elem << "\n";
            PrintDiagnostics(*elem);
        }
    }
    
    // std::string line = "test Of the Token 334 parser 3.1 haha \"this is awesome\" True";
    // TokenList l = LexLine(line);
    // std::cout << "######\n"; 
    // int pos=0;
    // Token* t;
    // for(t = NextTokenMatchingType(l, ObjectTokenTypes, pos); t != nullptr; t = NextTokenMatchingType(l, ObjectTokenTypes, pos))
    // {
    //     PrintDiagnostics(t);
    // }


    return 0;
}