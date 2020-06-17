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

// ---------------------------------------------------------------------------------------------------------------------
// Constructors

/// create a new scope with [inheritedScope]. all new scopes should be created from this
/// constructor method
Scope* ScopeConstructor(Scope* inheritedScope)
{
    Scope* s = new Scope;
    s->InheritedScope = inheritedScope;
    s->ReferencesIndex = {};

    return s;
}

/// creates a new block with [inheritedScope]. all new scopes should be created from this
/// constructor method
Block* BlockConstructor(Scope* inheritedScope)
{
    Block* b = new Block;
    b->ExecType = ExecutableType::Block;
    b->Executables = {};
    b->LocalScope = ScopeConstructor(inheritedScope);

    return b;
}



// ---------------------------------------------------------------------------------------------------------------------
// Diagnostics
/// logs scope
void LogDiagnosticsForRuntimeLine(Scope* scope, Operation* op)
{
    for(; scope != nullptr; scope=scope->InheritedScope)
        for(auto ref: scope->ReferencesIndex)
            LogDiagnostics(ref, MSG("scope before %s line %i", ToString(op->Type), op->LineNumber), "DoOperationOnReferences");
}


// ---------------------------------------------------------------------------------------------------------------------
// Scoping

/// Remove a reference from ObjectIndex of the global PROGRAM
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

/// Removes [ref] from [scope]
void RemoveReferenceFromScope(Scope* scope, Reference* ref)
{
    size_t refLoc;
    for(refLoc = 0; refLoc<scope->ReferencesIndex.size() && scope->ReferencesIndex.at(refLoc) != ref; refLoc++);
    scope->ReferencesIndex.erase(scope->ReferencesIndex.begin()+refLoc);
}

/// removes all dependencies on [ref] and deletes [ref]
void Dereference(Scope* scope, Reference* ref)
{
    LogItDebug(MSG("dereferenced: %s", ref->Name), "Dereference");
    RemoveReferenceFromObjectIndex(ref);
    RemoveReferenceFromScope(scope, ref);
    delete ref;
}

/// dereferences each element of [referencesList] from [scope]
void DereferenceAll(std::vector<Reference*> referenceList, Scope* scope)
{
    for(Reference* ref: referenceList)
    {
        if(ref->Name == c_returnReferenceName)
        {
            Dereference(scope, ref);
        }
    }
}








// ---------------------------------------------------------------------------------------------------------------------
// Program execution

/// executes an operation [op] on the ordered list of references [operands] inside [scope]
/// note: this method should only be called through DoOperation
Reference* DoOperationOnReferences(Scope* scope, Operation* op, std::vector<Reference*> operands)
{
    switch(op->Type)
    {
        case OperationType::Return:
        return OperationReturn(op->Value, scope);

        case OperationType::And:
        return OperationAnd(operands.at(0), operands.at(1), scope);

        case OperationType::Add:
        return OperationAdd(operands.at(0), operands.at(1), scope);

        case OperationType::Subtract:
        return OperationSubtract(operands.at(0), operands.at(1), scope);

        case OperationType::Print:
        return OperationPrint(operands.at(0), scope);

        case OperationType::Assign:
        return OperationAssign(op->Value, operands.at(0), scope);

        case OperationType::Define:
        return OperationDefine(op->Value, scope);

        case OperationType::If:
        return OperationIf(operands.at(0), scope);

        default:
        LogIt(LogSeverityType::Sev1_Notify, "DoOoperationOnReferences", "unimplemented in this case");
        return CreateNullReference();
    }
}

/// resolve the references return by each operand operation
std::vector<Reference*> GetOperandReferences(Scope* scope, Operation* op)
{
    std::vector<Reference*> operandReferences;
    
    for(Operation* operand: op->Operands)
    {
        Reference* operandRef = DoOperation(scope, operand);
        operandReferences.push_back(operandRef);
    }
    
    return operandReferences;
}

/// executes an operation [op] inside [scope]
Reference* DoOperation(Scope* scope, Operation* op)
{
    std::vector<Reference*> operandReferences = GetOperandReferences(scope, op);

    Reference* returnRef = DoOperationOnReferences(scope, op, operandReferences);
    
    DereferenceAll(operandReferences, scope);
    
    LogItDebug(MSG("line[%i] operation %s added a new reference to scope", op->LineNumber, ToString(op->Type)), "DoOperation");

    return returnRef;
}



void UpdatePreviousResult(Scope* scope, Reference** result, Reference** previousResult)
{
    if(*previousResult != nullptr)
        Dereference(scope, *previousResult);
    *previousResult = *result;
}

Block* TreatAsBlock(Executable* exec)
{
    return static_cast<Block*>(exec);
}

Operation* TreatAsOperation(Executable* exec)
{
    return static_cast<Operation*>(exec);
}

/// handles the If operation
Reference* HandleControlFlowIf(Operation* op, size_t& execLine, Scope* scope)
{
    Reference* ifExpressionResult = DoOperation(scope, op);
    if(op->Type == OperationType::If && !GetBoolValue(*ifExpressionResult->ToObject))
    {
        execLine++;
    }
    return ifExpressionResult;
}

/// executes [op] in [scope] and updates [execLine] based on the control flow properties
/// of [op]
Reference* HandleControlFlow(Operation* op, size_t& execLine, Scope* scope)
{
    switch(op->Type)
    {
        case OperationType::If:
        return HandleControlFlowIf(op, execLine, scope);

        // for any non-control flow operation;
        default:
        return DoOperation(scope, op);
    }
}

void HandleRuntimeMessages(int lineNumber)
{
    if(RuntimeMsgFlag)
    {
        RuntimeMsgPrint(lineNumber);
        RuntimeMsgFlag = false;
    }
}

/// executes the commands contained in a [codeBlock]
Reference* DoBlock(Block* codeBlock)
{
    Reference* result = nullptr;
    Reference* previousResult = nullptr;

    for(size_t i=0; i<codeBlock->Executables.size(); i++)
    {
        auto exec = codeBlock->Executables.at(i);

        if(exec->ExecType == ExecutableType::Operation)
        {
            Operation* op = TreatAsOperation(exec); 

            LogDiagnosticsForRuntimeLine(codeBlock->LocalScope, op);
            LogItDebug(MSG("starting execute line [%i]", op->LineNumber), "DoBlock");

            result = HandleControlFlow(op, i, codeBlock->LocalScope); 
            UpdatePreviousResult(codeBlock->LocalScope, &result, &previousResult);

            LogItDebug(MSG("finishes execute line [%i]", op->LineNumber), "DoBlock");

            HandleRuntimeMessages(op->LineNumber);

        }
        else if(exec->ExecType == ExecutableType::Block)
        {
            LogItDebug("discovered child block: starting" "DoBlock");

            result = DoBlock(TreatAsBlock(exec));

            LogItDebug("exiting child block", "DoBlock");

            // the result must be added to its parent scope
            AddReferenceToScope(result, codeBlock->LocalScope);
            UpdatePreviousResult(codeBlock->LocalScope, &result, &previousResult);
        }
    }

    if(result != nullptr)
        return result;
    else
        return CreateNullReference();
}

/// executes all blocks of [program]
void DoProgram(Program& program)
{
    for(Block* block: program.Blocks)
    {
        DoBlock(block);
    }
}








// ---------------------------------------------------------------------------------------------------------------------
// Reference matching

/// true if [name] refers to [ref]
bool NameMatchesReference(String name, Reference* ref)
{
    return name == ref->Name;
}

/// find a reference matching [refName] located in [scope]. returns nullptr if none found
Reference* GetReference(Scope* scope, String refName)
{
    for(Scope* lookInScope = scope; lookInScope != nullptr; lookInScope = lookInScope->InheritedScope)
    {
        for(Reference* ref: lookInScope->ReferencesIndex)
        {
            if(NameMatchesReference(refName, ref))
                return ref;
        }
    }
    ReportCompileMsg(SystemMessageType::Exception, MSG("cannot resolve reference [%s]", refName));
    return nullptr;
}

/// gets a reference for a primitive [value] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(int value, String name)
{
    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        Object* obj = map->Object;
        if(obj->Class == IntegerClass && GetIntValue(*obj) == value)
            return CreateReference(name, obj);
    }
    return CreateReferenceToNewObject(name, IntegerClass, value);
}

/// gets a reference for a primitive [value] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(double value, String name)
{
    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        Object* obj = map->Object;
        if(obj->Class == DecimalClass && GetDecimalValue(*obj) == value)
            return CreateReference(name, obj);
    }
    return CreateReferenceToNewObject(name, DecimalClass, value);
}

/// gets a reference for a primitive [value] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(bool value, String name)
{
    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        Object* obj = map->Object;
        if(obj->Class == BooleanClass && GetBoolValue(*obj) == value)
            return CreateReference(name, obj);
    }
    return CreateReferenceToNewObject(name, BooleanClass, value);
}

/// gets a reference for a primitive [value] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(String value, String name)
{
    for(ObjectReferenceMap* map: PROGRAM->ObjectsIndex)
    {
        Object* obj = map->Object;
        if(obj->Class == StringClass && GetStringValue(*obj) == value)
            return CreateReference(name, obj);
    }
    return CreateReferenceToNewObject(name, StringClass, value);
}

/// gets a reference for a primitive [token] with Name attribute [name]. if an existing primitive object
/// exists, the reference will point to that. otherwise a new object is created for the returned reference
Reference* ReferenceForPrimitive(Token* token, String name)
{
    if(!TokenMatchesType(token, PrimitiveTokenTypes))
        return nullptr;

    String value = token->Content;
    bool b;

    switch(token->Type)
    {
        case TokenType::Integer:
        return ReferenceForPrimitive(std::stoi(value), name);

        case TokenType::Boolean:
        b = value == "true" ? true : false;
        return ReferenceForPrimitive(b, name);

        case TokenType::String:
        return ReferenceForPrimitive(value, name);

        case TokenType::Decimal:
        return ReferenceForPrimitive(std::stod(value), name);

        default:
        LogIt(LogSeverityType::Sev3_Critical, "ReferenceForPrimitive", "unknown error");
        return nullptr;
    }
}



/// interface to getting, making, and assigning references from tokens
Reference* ReferenceFor(Token* token, Scope* scope, String refName)
{
    if(TokenMatchesType(token, PrimitiveTokenTypes))
    {
        return ReferenceForPrimitive(token, refName);
    }
    else if(TokenMatchesType(token, TokenType::Reference))
    {
        return GetReference(scope, token->Content);
    }
    LogIt(Sev3_Critical, "ReferenceFor", "unknown error");
    return CreateNullReference();
}



Reference* DefineNewReference(String refName, int value, Scope* scope)
{
    // TODO: currently only accepts primitives;
    Reference* ref = ReferenceForPrimitive(value, refName);
    ref->Name = refName;

    AddReferenceToScope(ref, scope);

    return ref;
}

Reference* DefineNewReference(String refName, bool value, Scope* scope)
{
    // TODO: currently only accepts primitives;
    Reference* ref = ReferenceForPrimitive(value, refName);
    ref->Name = refName;

    AddReferenceToScope(ref, scope);

    return ref;
}

Reference* DefineNewReference(String refName, String value, Scope* scope)
{
    // TODO: currently only accepts primitives;
    Reference* ref = ReferenceForPrimitive(value, refName);
    ref->Name = refName;

    AddReferenceToScope(ref, scope);

    return ref;
}

Reference* DefineNewReference(String refName, double value, Scope* scope)
{
    // TODO: currently only accepts primitives;
    Reference* ref = ReferenceForPrimitive(value, refName);
    ref->Name = refName;

    AddReferenceToScope(ref, scope);

    return ref;
}


/// defines a new reference with name attribute [refName] inside [scope]
Reference* DefineNewReference(String refName, Token* valueToken, Scope* scope)
{
    // TODO: currently only accepts primitives;
    Reference* ref = ReferenceForPrimitive(valueToken, refName);
    ref->Name = refName;

    AddReferenceToScope(ref, scope);

    return ref;
}

/// defines a new reference with name attribute [refName] to [obj] inside [scope] 
Reference* DefineNewReference(String refName, Object* obj, Scope* scope)
{
    Reference* ref = CreateReference(refName, obj);
    AddReferenceToScope(ref, scope);
    return ref;
}

Reference* DefineNullReference(String refName, Scope* scope)
{
    Reference* ref = CreateNullReference(refName);
    AddReferenceToScope(ref, scope);
    return ref;
}

Reference* DefineNullReference(Scope* scope)
{
    Reference* ref = CreateNullReference();
    AddReferenceToScope(ref, scope);
    return ref;
}

/// add [ref] to [scope]
void AddReferenceToScope(Reference* ref, Scope* scope)
{
    scope->ReferencesIndex.push_back(ref);
}

void ReassignReference(Reference* ref, Object* newObj)
{
    RemoveReferenceFromObjectIndex(ref);
    IndexObject(newObj, ref);
    ref->ToObject = newObj;
}





/// given a Token* [token], will return either an existing reference (or null reference if none match)
/// or a new reference to a primitive object
// Reference* DecideReferenceOf(Scope* scope, Token* token)
// {
//     if(token == nullptr)
//         return CreateNullReference();

//     Reference* tokenRef;

//     tokenRef = DecideExistingReferenceFor(token, scope);
//     if(tokenRef != nullptr)
//     {
//         LogItDebug("found existing object", "DecideReferenceOf");
//         return tokenRef;
//     }

//     tokenRef = GetReferenceForPrimitive(token, c_returnReferenceName);
//     if(tokenRef != nullptr)
//     {
//         LogItDebug("created a new object", "DecideReferenceOf");
//         return tokenRef;
//     }


//     return CreateNullReference();
// }



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
    if(FindToken(tokens, "if") != nullptr && FindToken(tokens, ":") != nullptr)
        lineType = LineType::IfLine;
    else
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

String g_tabString;

bool TabStringIsSet()
{
    return g_tabString.size() > 0;
}

void SetTabString(String str)
{
    if(str.size() == 0)
        return;
    if(TabStringIsSet())
        return;

    g_tabString = str;
}

bool IsTabCharacter(char c)
{
    return c == '\t' || c == ' ';
}

String DecideTabString(String line)
{
    String tabString;
    tabString.reserve(8);

    for(size_t i =0; i < line.size() && IsTabCharacter(line.at(i)); i++)
        tabString += line.at(i);
    
    return tabString;
}

/// returns the level of the line or -1 if malformed
int LevelOfLine(String line)
{
    if(line.size() == 0)
        return -1;
    if(!IsTabCharacter(line.at(0)))
        return 0;

    // if some spacing exists but the tabString is not set, the code is malformed
    if(g_tabString.size() == 0)
        return -1;

    int level=0;
    size_t i=0;
    while(static_cast<size_t>(level) < line.size())
    {
        for(size_t j=0; j<g_tabString.size() && i<line.size(); j++, i++)
        {
            if(g_tabString.at(j) != line.at(i)){
                return -1;
            }
        }
        level++;
        if(!IsTabCharacter(line.at(i)))
            return level;
    }
    return -1;
}



/// returns a line of code and sets lineNumber to that of the next line and lineStart to
/// the starting position of the returned line. commas allw for a line to be split
std::string GetEffectiveLine(std::fstream& file, int& lineNumber, int& lineStart)
{
    std::string fullLine = "";
    std::string newLine;

    bool FirstTime = true;
    do
    {
        lineNumber++;
        if(!std::getline(file, newLine))
            break;
        if(newLine != "" && FirstTime)
        {
            lineStart = lineNumber - 1;
            FirstTime = false;
        }
        fullLine += newLine;

    } while (newLine.size() == 0 || LastNonWhitespaceChar(newLine) == ',');

    if(!TabStringIsSet())
        SetTabString(DecideTabString(newLine));

    return RemoveCommas(fullLine);
}


/// parses an atomic operation into an Operation tree
Operation* ParseOutAtomic(Scope* scope, PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    OperationType opType;
    DecideOperationType(typeProbabilities, opType);

    LogItDebug(MSG("operation type is %s", ToString(opType)), "ParseOutAtomic");
    // these operands act on references!
    Reference* refValue = nullptr;
    if(opType == OperationType::Return || opType == OperationType::Define || opType == OperationType::Assign)
    {
        DecideOperationValue(scope, opType, tokens, &refValue);
    }

    OperationsList operands;
    DecideOperands(scope, opType, tokens, operands);

    Operation* op = OperationConstructor(opType, operands, refValue);

    return op;
}

// TODO: Implement
/// parses a composite operation into an Operation tree
Operation* ParseComposite(Scope* scope, PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    
    return nullptr;
}

// TODO:
Operation* ParseIf(Scope* scope, PossibleOperationsList& typeProbabilityes, TokenList& tokens)
{
    // Token* condition = NextTokenMatching(tokens, ObjectTokenTypes);

    TokenList newList;
    newList = RightOfToken(tokens, FindToken(tokens, "if"));
    newList = LeftOfToken(newList, FindToken(newList, ":"));

    LogDiagnostics(newList, "printing token list after removing if stuff");

    Operation* condition = ParseLine(scope, newList);
    Operation* op = OperationConstructor(
        OperationType::If, 
        { condition });

    return op;
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

        case LineType::IfLine:
        return ParseIf(scope, typeProbabilities, tokens);

        case LineType::While:

        default:
        LogIt(LogSeverityType::Sev1_Notify, "ParseLine", "unimplemented in case");
        return nullptr;
    }
}

int SizeOfBlock(std::vector<CodeLine>::iterator it, std::vector<CodeLine>::iterator end)
{
    int blockLevel = it->Level;
    int blockSize = 0;
    for(; it != end && it->Level >= blockLevel; it++) 
        blockSize++;

    return blockSize;
}

bool InsideChildBlock(std::vector<CodeLine>::iterator it, int previousLineLevel)
{
    return it->Level > previousLineLevel;
}


Block* ParseBlock(
    std::vector<CodeLine>::iterator it, 
    std::vector<CodeLine>::iterator end, 
    Scope* blockInheritedScope,
    Scope* compileTimeInheritedScope)
{
    Block* thisBlock = BlockConstructor(blockInheritedScope);
    Scope* thisBlockCompileScope = ScopeConstructor(compileTimeInheritedScope);

    int previousLineLevel = it->Level;

    for(; it != end; it++)
    {
        if(InsideChildBlock(it, previousLineLevel))
        {
            LogItDebug(MSG("starting compile new block at line [%i]", it->LineNumber), "ParseBlock");
            int blockSize = SizeOfBlock(it, end);
            Block* b = ParseBlock(it, it+blockSize, thisBlock->LocalScope, thisBlockCompileScope);
            LogItDebug(MSG("finishes compile new block at line [%i]", it->LineNumber), "ParseBlock");

            // increment iterator to end of block
            it += blockSize - 1;
            thisBlock->Executables.push_back(b);
        }
        else
        {
            LogItDebug(MSG("starting compile line [%i]", it->LineNumber), "ParseBlock");
            Operation* op = ParseLine(thisBlockCompileScope, it->Tokens);
            NumberOperation(op, it->LineNumber);
            LogItDebug(MSG("finishes compile line [%i]", it->LineNumber), "ParseBlock");

            if(CompileMsgFlag)
            {
                CompileMsgPrint(it->LineNumber);
                CompileMsgFlag = false;
            }

            thisBlock->Executables.push_back(op);
        }
    }

    return thisBlock;
}

Program* ParseProgram(const std::string filepath)
{
    PROGRAM = new Program;
    PROGRAM->GlobalScope = ScopeConstructor(nullptr);
    Scope* compileTimeGlobalScope = ScopeConstructor(nullptr);

    std::fstream file;
    file.open(filepath, std::ios::in);

    int lineLevel = 0;
    int nextLinePos = 1;
    int lineStart = 1;

    for(std::string line = GetEffectiveLine(file, nextLinePos, lineStart); line != ""; line = GetEffectiveLine(file, nextLinePos, lineStart))
    {
        TokenList tokens = LexLine(line);
        lineLevel = LevelOfLine(line);

        CodeLine ls = { tokens , lineStart, lineLevel };
        PROGRAM->Lines.push_back(ls);
    }

    // TODO: Allow different blocks
    Block* b = ParseBlock(PROGRAM->Lines.begin(), PROGRAM->Lines.end(), PROGRAM->GlobalScope, compileTimeGlobalScope);
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
    
    for(Block* b: PROGRAM->Blocks)
        LogDiagnostics(b, "initial program parse structure", "main");
    
    
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
    
    // std::string line = "test Of the Token: ;::;4 : 334 par.ser=4 &.&.3&&& = 3.1 haha \"this is awesome\" True";
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
