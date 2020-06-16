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



Reference* OperationReturn(Reference* ref, Scope* scope)
{
    return CreateReference(c_returnReferenceName, ref->ToObject);
}





// Program execution

/// executes an operation [op] on the ordered list of reference [operands]. should only be called
/// through DoOperation
Reference* DoOperationOnReferences(Scope* scope, Operation* op, std::vector<Reference*> operands)
{
    switch(op->Type)
    {
        case OperationType::Return:
        return OperationReturn(op->Value, scope);

        case OperationType::Add:
        return OperationAdd(operands.at(0), operands.at(1));

        case OperationType::Subtract:
        return OperationSubtract(operands.at(0), operands.at(1));

        case OperationType::Print:
        return OperationPrint(operands.at(0));

        case OperationType::Assign:
        return OperationAssign(op->Value, operands.at(0));
  

        case OperationType::Define:
        return OperationDefine(op->Value, scope);

        default:
        LogIt(LogSeverityType::Sev1_Notify, "DoOoperationOnReferences", "unimplemented in this case");
        return CreateNullReference();
    }
}

/// executes an operation [op]
Reference* DoOperation(Scope* scope, Operation* op)
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
            LogItDebug(MSG("line[%i] operation %s dereferenced %s", op->LineNumber, ToString(op->Type), ref->Name), "DoOperation");
            Dereference(scope, ref);
        }
    }
    LogItDebug(MSG("line[%i] operation %s added a new reference to scope", op->LineNumber, ToString(op->Type)), "DoOperation");

    AddReferenceToScope(returnRef, scope);
    return returnRef;
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

        if(previousResult != nullptr)
            Dereference(codeBlock.LocalScope, previousResult);
        previousResult = result;
        LogItDebug(MSG("finishes execute line [%i]", op->LineNumber), "DoBlock");

        for(auto ref: codeBlock.LocalScope->ReferencesIndex)
            LogDiagnostics(ref, MSG("scope at %s line %i", ToString(op->Type), op->LineNumber), "DoOperationOnReferences");

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

bool TokenMatchesPrimitive(Token* token, Reference* ref)
{
    return token->Content == GetStringValue(*ref->ToObject);
}

/// returns a pointer to a Reference if [token] corresponds to an existing reference and nullptr if 
/// no matching token can be resolved;
Reference* DecideExistingReferenceFor(Token* token, Scope* scope) // MAJOR: Add scope
{
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
        return CreateReferenceToNewObject(c_primitiveObjectName, token);
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
    {
        LogItDebug("found existing object", "DecideReferenceOf");
        return tokenRef;
    }

    tokenRef = DecideNewReferenceFor(token);
    if(tokenRef != nullptr)
    {
        LogItDebug("created a new object", "DecideReferenceOf");
        return tokenRef;
    }


    return CreateNullReference();
}



// can put this somewhere
typedef void (*ProbabilityFunctions)(PossibleOperationsList&, const TokenList&);
ProbabilityFunctions decideProbabilities[] = 
{
    DecideProbabilityDefine,
    DecideProbabilityAssign,
    DecideProbabilityIsEqual,
    DecideProbabilityIsLessThan,
    DecideProbabilityIsGreaterThan,
    DecideProbabilityAdd,
    DecideProbabilitySubtract,
    DecideProbabilityMultiply,
    DecideProbabilityDivide,
    DecideProbabilityAnd,
    DecideProbabilityOr,
    DecideProbabilityNot,
    DecideProbabilityEvaluate,
    DecideProbabilityPrint,
    DecideProbabilityReturn,
};

/// decide the probability that a line represented by [tokens] corresponds to each of the atomic operations and stores
/// this in [typeProbabilities]
void DecideOperationTypeProbabilities(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    for(ProbabilityFunctions pFunc : decideProbabilities)
    {
        pFunc(typeProbabilities, tokens);
    }
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


typedef void(*DecideOperandsFunction)(Scope*, TokenList& tokens, OperationsList&);
DecideOperandsFunction decideOperands[] = 
{
    DecideOperandsDefine,
    DecideOperandsAssign,
    DecideOperandsIsEqual,
    DecideOperandsIsLessThan,
    DecideOperandsIsGreaterThan,
    DecideOperandsAdd,
    DecideOperandsSubtract,
    DecideOperandsMultiply,
    DecideOperandsDivide,
    DecideOperandsAnd,
    DecideOperandsOr,
    DecideOperandsNot,
    DecideOperandsEvaluate,
    DecideOperandsPrint,
    DecideOperandsReturn,
};

/// decides and adds the operations for the Operation of [opType] to [operands] 
void DecideOperands(Scope* scope, const OperationType& opType, TokenList& tokens, OperationsList& operands)
{
    // TODO fPtr
    // makes going through a linear search to find what function to use instant with array indexing
    decideOperands[opType](scope, tokens, operands);
}


//
typedef void(*DecideValueFunctions)(Scope*, TokenList&, Reference**);
DecideValueFunctions valueFunctions[] = 
{
    DecideValueDefine,
    DecideValueAssign,
    DecideValueIsEqual,
    DecideValueIsLessThan,
    DecideValueIsGreaterThan,
    DecideValueAdd,
    DecideValueSubtract,
    DecideValueMultiply,
    DecideValueDivide,
    DecideValueAnd,
    DecideValueOr,
    DecideValueNot,
    DecideValueEvaluate,
    DecideValuePrint,
    DecideValueReturn,
}; // need to add the rest of the functions pointers for it to be good
//
void DecideOperationValue(Scope* scope, const OperationType& opType, TokenList& tokens, Reference** refValue)
{
    valueFunctions[opType](scope, tokens, refValue);
}



char LastNonWhitespaceChar(String& line)
{
    int i = line.size();
    while (i--, i >= 0 && line.at(i) == ' ');
    return (i >= 0 ? line.at(i) : '\0');
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

    } while (newLine.size() == 0 || LastNonWhitespaceChar(newLine) == ',');
    return RemoveCommas(fullLine);
}




/// parses an atomic operation into an Operation tree
Operation* ParseOutAtomic(Scope* scope, PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    Operation* op = CreateOperation();

    OperationType opType;
    DecideOperationType(typeProbabilities, opType);
    op->Type = opType;

    LogItDebug(MSG("operation type is %s", ToString(opType)), "ParseOutAtomic");
    // these operands act on references!
    if(opType == OperationType::Return || opType == OperationType::Define || opType == OperationType::Assign)
    {
        Reference* refValue;
        DecideOperationValue(scope, opType, tokens, &refValue);
        op->Value = refValue;
    }

    OperationsList operands;
    DecideOperands(scope, opType, tokens, operands);
    op->Operands = operands;

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



// can use this if every plays 
typedef Operation*(*LineTypeFunctions)(Scope*, PossibleOperationsList&, TokenList&);
LineTypeFunctions lineFunctions[] = {ParseOutAtomic, ParseComposite};

/// parses a line of code
Operation* ParseLine(Scope* scope, TokenList& tokens)
{
    PossibleOperationsList typeProbabilities;
    DecideOperationTypeProbabilities(typeProbabilities, tokens);

    LineType lineType;
    DecideLineType(typeProbabilities, tokens, lineType);
    
    // fPtr
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

Block ParseBlock(
    std::vector<CodeLine>::iterator it, 
    std::vector<CodeLine>::iterator end, 
    Scope* inheritedScope)
{
    Block parsedBlock;
    parsedBlock.LocalScope = new Scope;
    parsedBlock.LocalScope->InheritedScope = inheritedScope;

    Scope* compileTimeBlockScope = new Scope;

    for(; it != end; it++)
    {
        LogItDebug(MSG("starting compile line [%i]", it->LineNumber), "ParseBlock");
        Operation* op = ParseLine(compileTimeBlockScope, it->Tokens);
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
    
    for(auto op: PROGRAM->Blocks.at(0).Operations)
        LogDiagnostics(op, "operation log", "main");

    PROGRAM->Blocks.at(0).LocalScope->ReferencesIndex.clear();
    
    // PRINT OPERATIONS
    if(PRINT_OPERATIONS)
    {
    }


    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        LogDiagnostics(map, "initial object reference state", "main");
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

    LogItDebug("end reached.", "main");
    return 0;
}