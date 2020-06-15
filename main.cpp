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
#include "program.h"
#include "token.h"
#include "object.h"
#include "diagnostics.h"
#include "operation.h"

// Scoping
void RemoveReferenceFromObjectIndex(Reference* ref)
{
    ObjectReferenceMap* map = EntryInIndexOf(ref->ToObject);
    if(map == nullptr)
    {
        LogIt(LogSeverityType::Sev3_Critical, "RemoveReferenceFromObjectIndex", "cannot find reference in ObjectIndex");
        return;
    }

    size_t refLoc;
    for(refLoc=0; refLoc<map->References.size() && ref != map->References.at(refLoc); refLoc++);
    map->References.erase(map->References.begin()+refLoc);
}

void RemoveReferenceFromScope(Scope* scope, Reference* ref)
{
    size_t refLoc;
    for(refLoc = 0; refLoc<scope->ReferencesIndex.size() && scope->ReferencesIndex.at(refLoc) != ref; refLoc++);
    scope->ReferencesIndex.erase(scope->ReferencesIndex.begin()+refLoc);
}

void Dereference(Scope* scope, Reference* ref)
{
    RemoveReferenceFromObjectIndex(ref);
    RemoveReferenceFromScope(scope, ref);
}









// Program execution

/// executes an operation [op] on the ordered list of reference [operands]. should only be called
/// through DoOperation
Reference* DoOperationOnReferences(Scope* scope, Operation* op, std::vector<Reference*> operands)
{
    Reference* result;
    switch(op->Type)
    {
        case OperationType::Add:
        return OperationAdd(operands.at(0), operands.at(1));

        case OperationType::Print:
        return OperationPrint(operands.at(0));

        case OperationType::Assign:
        // Dereference(scope, operands.at(0));
        result = OperationAssign(operands.at(0), operands.at(1));
        // Dereference(scope, operands.at(1));
        // AddReferenceToScope(operands.at(0), scope);
        for(auto ref: scope->ReferencesIndex)
            LogDiagnostics(ref, "scope here", "DoOperationOnReferences");

        return result;

        case OperationType::Define:
        return OperationDefine(op->Value);

        default:
        LogIt(LogSeverityType::Sev1_Notify, "DoOoperationOnReferences", "unimplemented in this case");
        return CreateNullReference();
    }
}

/// executes an operation [op]
Reference* DoOperation(Scope* scope, Operation* op)
{
    if(op->Type == OperationType::Return)
    {
        LogDiagnostics(op->Value, "added a reference", "DoOperation");
        AddReferenceToScope(op->Value, scope);
        return op->Value;
    }
    else
    {
        std::vector<Reference*> operandReferences;
        for(Operation* operand: op->Operands)
        {
            Reference* operandRef = DoOperation(scope, operand);
            operandReferences.push_back(operandRef);
        }
        
        Reference* returnRef = DoOperationOnReferences(scope, op, operandReferences);
        
        for(Reference* ref: operandReferences)
        {
            if(ref->Name == c_returnReferenceName)
            {
                Dereference(scope, ref);
            }
        }
        AddReferenceToScope(returnRef, scope);
        return returnRef;
    }
}

/// executes a [codeBlock]
void DoBlock(Block& codeBlock)
{
    Reference* result = nullptr;
    Reference* previousResult = nullptr;

    for(Operation* op: codeBlock.Operations)
    {
        LogItDebug(MSG("starting execute line [%i]", op->LineNumber), "DoBlock");
        Reference* result = DoOperation(codeBlock.LocalScope, op);
        LogItDebug(MSG("finishes execute line [%i]", op->LineNumber), "DoBlock");
        
   
        if(previousResult != nullptr)
            Dereference(codeBlock.LocalScope, previousResult);
        previousResult = result;

        if(RuntimeMsgFlag)
        {
            RuntimeMsgPrint(op->LineNumber);
            RuntimeMsgFlag = false;
        }
    }
    if(result != nullptr)
        Dereference(codeBlock.LocalScope, result);
}

void DoProgram(Program& program)
{
    for(Block block: program.Blocks)
    {
        DoBlock(block);
    }
}







// Meta Parsing + Deciding
/// returns true if [token] corresponds to [ref]
bool TokenMatchesReference(Token* token, Reference* ref) // MAJOR
{
    return token->Content == ref->Name;
}

/// returns a pointer to a Reference if [token] corresponds to an existing reference and nullptr if 
/// no matching token can be resolved;
Reference* DecideExistingReferenceFor(Token* token, Scope* scope) // MAJOR: Add scope
{
    // URGENT
    if(TokenMatchesType(token, TokenType::Reference))
    {
        for(Scope* lookInScope = scope; lookInScope != nullptr; lookInScope = lookInScope->InheritedScope)
        {
            for(Reference* ref: scope->ReferencesIndex)
            {
                if(TokenMatchesReference(token, ref))
                    return ref;
            }
        }
        ReportCompileMsg(SystemMessageType::Exception, MSG("cannot resolve reference [%s]", token->Content));
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
Reference* DecideReferenceOf(Scope* scope, Token* token)
{
    if(token == nullptr)
        return CreateNullReference();

    Reference* tokenRef;

    tokenRef = DecideExistingReferenceFor(token, scope);
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
void DecideOperands(Scope* scope, const OperationType& opType, TokenList& tokens, OperationsList& operands)
{
    switch(opType)
    {
        case OperationType::Define:
        DecideOperandsDefine(scope, tokens, operands);
        break;

        case OperationType::Assign:
        DecideOperandsAssign(scope, tokens, operands);
        break;

        case OperationType::IsEqual:
        DecideOperandsIsEqual(scope, tokens, operands);
        break;

        case OperationType::IsLessThan:
        DecideOperandsIsLessThan(scope, tokens, operands);
        break;

        case OperationType::IsGreaterThan:
        DecideOperandsIsGreaterThan(scope, tokens, operands);
        break;

        case OperationType::Add:
        DecideOperandsAdd(scope, tokens, operands);
        break;

        case OperationType::Subtract:
        DecideOperandsSubtract(scope, tokens, operands);
        break;

        case OperationType::Multiply:
        DecideOperandsMultiply(scope, tokens, operands);
        break;

        case OperationType::Divide:
        DecideOperandsDivide(scope, tokens, operands);
        break;

        case OperationType::And:
        DecideOperandsAnd(scope, tokens, operands);
        break;

        case OperationType::Or:
        DecideOperandsOr(scope, tokens, operands);
        break;

        case OperationType::Not:
        DecideOperandsNot(scope, tokens, operands);
        break;

        case OperationType::Evaluate:
        DecideOperandsEvaluate(scope, tokens, operands);
        break;

        case OperationType::Print:
        DecideOperandsPrint(scope, tokens, operands);
        break;

        case OperationType::Return:
        DecideOperandsReturn(scope, tokens, operands);
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
/// returns a line of code and sets lineNumber to that of the next line and lineStart to
/// the starting position of the returned line
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
Operation* ParseOutAtomic(Scope* scope, PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    OperationType opType;
    DecideOperationType(typeProbabilities, opType);

    OperationsList operands;
    DecideOperands(scope, opType, tokens, operands);

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
Operation* ParseComposite(Scope* scope, PossibleOperationsList& typeProbabilities, TokenList& tokens)
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
Operation* ParseLine(Scope* scope, TokenList& tokens)
{
    PossibleOperationsList typeProbabilities;
    DecideOperationTypeProbabilities(typeProbabilities, tokens);

    LineType lineType;
    DecideLineType(typeProbabilities, tokens, lineType);
    
    switch(lineType)
    {
        case LineType::Atomic:
        return ParseOutAtomic(scope, typeProbabilities, tokens);

        case LineType::Composite:
        return ParseComposite(scope, typeProbabilities, tokens);

        case LineType::If:

        case LineType::While:

        default:
        LogIt(LogSeverityType::Sev1_Notify, "ParseLine", "unimplemented in case");
        return nullptr;
    }
}

/// parses a block of code
// Block ParseBlock(const std::string filepath, int& lineNumber){
//     Block parsedBlock;
//     std::fstream file;
//     file.open(filepath, std::ios::in);

//     int lineEndPos = lineNumber;
//     int lineStart = lineNumber;
//     for(std::string line = GetEffectiveLine(file, lineEndPos, lineStart); line != ""; line = GetEffectiveLine(file, lineEndPos, lineStart))
//     {
//         TokenList tokens = LexLine(line);
//         Operation* op = ParseLine(tokens);

//         if(CompileMsgFlag)
//         {
//             CompileMsgPrint(lineStart);
//             CompileMsgFlag = false;
//         }

//         NumberOperation(op, lineStart);
//         parsedBlock.Operations.push_back(op);

//     }

//     return parsedBlock;
// }

Block ParseBlock(std::vector<CodeLine>::iterator it, std::vector<CodeLine>::iterator end, Scope* inheritedScope)
{
    Block parsedBlock;
    parsedBlock.LocalScope = new Scope;
    parsedBlock.LocalScope->InheritedScope = inheritedScope;
    parsedBlock.LocalScope->ReferencesIndex = {};

    for(; it != end; it++)
    {
        LogItDebug(MSG("starting compile line [%i]", it->LineNumber), "ParseBlock");
        Operation* op = ParseLine(parsedBlock.LocalScope, it->Tokens);
        NumberOperation(op, it->LineNumber);
        LogItDebug(MSG("finishes compile line [%i]", it->LineNumber), "ParseBlock");

        if(CompileMsgFlag)
        {
            CompileMsgPrint(it->LineNumber);
            CompileMsgFlag = false;
        }

        parsedBlock.Operations.push_back(op);
    }

    return parsedBlock;
}

Program* ParseProgram(const std::string filepath)
{
    PROGRAM = new Program;
    PROGRAM->GlobalScope = new Scope;
    PROGRAM->GlobalScope->InheritedScope = nullptr;


    std::fstream file;
    file.open(filepath, std::ios::in);

    int lineEndPos = 1;
    int lineStart = 1;
    for(std::string line = GetEffectiveLine(file, lineEndPos, lineStart); line != ""; line = GetEffectiveLine(file, lineEndPos, lineStart))
    {
        TokenList tokens = LexLine(line);
        CodeLine ls = { tokens , lineStart };
        PROGRAM->Lines.push_back(ls);
    }

    // TODO: Allow different blocks
    Block b = ParseBlock(PROGRAM->Lines.begin(), PROGRAM->Lines.end(), PROGRAM->GlobalScope);
    PROGRAM->Blocks.push_back(b);

    return PROGRAM;
}




int main()
{
    PurgeLog();

    bool PRINT_OPERATIONS = false;
    bool PRINT_GLOBAL_REFS = false;
    LogIt(LogSeverityType::Sev1_Notify, "main", "program compile begins");
    ParseProgram(".\\program");
    LogIt(LogSeverityType::Sev1_Notify, "main", "program compile finished");
    
    PROGRAM->Blocks.at(0).LocalScope->ReferencesIndex.clear();
    
    // PRINT OPERATIONS
    if(PRINT_OPERATIONS)
    {
    }

    // for(auto elem: PROGRAM->Blocks.at(0).LocalScope->ReferencesIndex)
    //     LogDiagnostics(elem);

    std::cout << "####################\n";
    LogIt(LogSeverityType::Sev1_Notify, "main", "program execution begins");
    DoProgram(*PROGRAM);
    LogIt(LogSeverityType::Sev1_Notify, "main", "program execution finished");
    std::cout << "####################\n";
    
    // PRINT GLOBALREFRENCES
    if(PRINT_GLOBAL_REFS)
    {
        // for(auto elem: GlobalScope.ReferencesIndex)
        // {
        //     std::cout << &elem << "\n";
        //     LogDiagnostics(*elem);
        // }
    }
    
    // std::string line = "test Of the Token 334 parser = 3.1 haha \"this is awesome\" True";
    // TokenList l = LexLine(line);
    // std::cout << "######\n"; 
    // int pos=0;
    // Token* t;
    // for(t = NextTokenMatching(l, ObjectTokenTypes, pos); t != nullptr; t = NextTokenMatching(l, ObjectTokenTypes, pos))
    // {
    //     LogDiagnostics(t);
    // }
    // LogDiagnostics(l);

    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        LogDiagnostics(map, "final object reference state", "main");
    }


    return 0;
}