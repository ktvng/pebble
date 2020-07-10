#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "main.h"
#include "program.h"
#include "token.h"
#include "diagnostics.h"
#include "operation.h"
#include "reference.h"
#include "decision.h"
#include "parse.h"
#include "object.h"
#include "execute.h"
#include "scope.h"

/// program to execute
Program* PROGRAM;

/// override to use static grammar
bool override = true;

// contain output (for testing/diagnostic purposes)
std::string ProgramOutput;
std::string ProgramMsgs;

// Error reporting
std::vector<SystemMessage> RuntimeMsgBuffer;
std::vector<SystemMessage> CompileMsgBuffer;

/// flag which is true whenever the RuntimeMsgBuffer contains a pending error message
bool RuntimeMsgFlag = false;
bool CompileMsgFlag = false;

bool FatalCompileError = false;

int RuntimeMsgCount = 0;
int CompileMsgCount = 0;

/// adds an error to the error buffer which will be printed when RuntimeErrorPrint is called
void ReportRuntimeMsg(SystemMessageType type, String message)
{
    SystemMessage msg { message, type };
    RuntimeMsgBuffer.push_back(msg);
    RuntimeMsgFlag = true;
    RuntimeMsgCount++;
}

void ReportCompileMsg(SystemMessageType type, String message)
{
    SystemMessage msg { message, type };
    CompileMsgBuffer.push_back(msg);
    CompileMsgFlag = true;
    CompileMsgCount++;
}



// ---------------------------------------------------------------------------------------------------------------------
// Constructors

/// creates a new block with [inheritedScope]. all new scopes should be created from this
/// constructor method
Block* BlockConstructor()
{
    Block* b = new Block;
    b->ExecType = ExecutableType::Block;
    b->Executables = {};

    return b;
}

void BlockDestructor(Block* b)
{
    delete b;
}

void Reset()
{
    ProgramOutput.clear();
    ProgramMsgs.clear();
    RuntimeMsgBuffer.clear();
    CompileMsgBuffer.clear();
    RuntimeMsgFlag = false;
    CompileMsgFlag = false;
    FatalCompileError = false;
    RuntimeMsgCount = 0;
    CompileMsgCount = 0;
}

void EnterProgram(Program* p)
{
    PROGRAM = p;
}

void ExitProgram()
{
    PROGRAM = nullptr;
}

Program* ProgramConstructor()
{
    Program* p = new Program;
    p->GlobalScope = ScopeConstructor(nullptr);

    EnterProgram(p);
    EnterScope(p->GlobalScope);
    {
        Reference* ref = CreateReferenceToNewObject("Object", BaseClass, nullptr);
        AddReferenceToCurrentScope(ref);
    }
    ExitScope();
    ExitProgram();

    return p;
}






// ---------------------------------------------------------------------------------------------------------------------
// ParseHelpers

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

inline const std::string SpaceChars = " \n\t";

bool CharIsWhiteSpace(char c)
{
    for(size_t i=0; i<SpaceChars.size(); i++)
    {
        if(c == SpaceChars[i])
            return true;
    }
    return false;
}

bool LineIsWhitespace(std::string& line)
{
    for(size_t i =0; i<line.size(); i++)
    {
        if(!CharIsWhiteSpace(line[i]))
            return false;
    }
    return true;
}


/// returns a line of code and sets lineNumber to that of the next line and lineStart to
/// the starting position of the returned line. commas allow for a line to be split
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

        /// skip lines of whitespace
        if(LineIsWhitespace(newLine))
            continue;

        if(FirstTime)
        {
            lineStart = lineNumber - 1;
            FirstTime = false;
        }
        fullLine += newLine;

    } while (LineIsWhitespace(newLine) || LastNonWhitespaceChar(newLine) == ',');

    if(!TabStringIsSet())
        SetTabString(DecideTabString(newLine));

    return fullLine;
    // return RemoveCommas(fullLine);
}

// ---------------------------------------------------------------------------------------------------------------------
// Parsing

/// parses an atomic operation into an Operation tree
Operation* ParseOutAtomic(PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    OperationType opType;
    DecideOperationType(typeProbabilities, opType);

    LogItDebug(Msg("operation type is %s", ToString(opType)), "ParseOutAtomic");
    // these operands act on references!
    Reference* refValue = nullptr;
    if(opType == OperationType::Ref)
    {
        DecideValueRef(tokens, &refValue);
    }

    OperationsList operands;
    DecideOperands(opType, tokens, operands);

    Operation* op = OperationConstructor(opType, operands, refValue);

    return op;
}

// TODO: Implement
/// parses a composite operation into an Operation tree
Operation* ParseComposite(PossibleOperationsList& typeProbabilities, TokenList& tokens)
{
    Operation* op = ExpressionParser(tokens);
    return op;
}

// TODO:
Operation* ParseIf(PossibleOperationsList& typeProbabilityes, TokenList& tokens)
{
    // Token* condition = NextTokenMatching(tokens, ObjectTokenTypes);

    TokenList newList;
    newList = RightOfToken(tokens, FindToken(tokens, "if"));
    newList = LeftOfToken(newList, FindToken(newList, ":"));

    LogDiagnostics(newList, "printing token list after removing if stuff");

    Operation* condition = ParseLine(newList);
    Operation* op = OperationConstructor(OperationType::If, { condition });

    return op;
}

Operation* ParseWhile(PossibleOperationsList& typeProbabilityes, TokenList& tokens)
{
    // Token* condition = NextTokenMatching(tokens, ObjectTokenTypes);

    TokenList newList;
    newList = RightOfToken(tokens, FindToken(tokens, "while"));
    newList = LeftOfToken(newList, FindToken(newList, ":"));

    LogDiagnostics(newList, "printing token list after removing if stuff");

    Operation* condition = ParseLine(newList);
    Operation* op = OperationConstructor(OperationType::While, { condition });

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
typedef Operation*(*LineTypeFunctions)(PossibleOperationsList&, TokenList&);
LineTypeFunctions lineFunctions[] = {ParseOutAtomic, ParseComposite};

/// parses a line of code
Operation* ParseLine(TokenList& tokens)
{
    PossibleOperationsList typeProbabilities;
    DecideOperationTypeProbabilities(typeProbabilities, tokens);

    LineType lineType;
    DecideLineType(typeProbabilities, tokens, lineType);
    
    // fPtr
    switch(lineType)
    {
        case LineType::Atomic:
        return ParseOutAtomic(typeProbabilities, tokens);

        case LineType::Composite:
        return ParseComposite(typeProbabilities, tokens);

        case LineType::IfLine:
        return ParseIf(typeProbabilities, tokens);

        case LineType::WhileLine:
        return ParseWhile(typeProbabilities, tokens);

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

bool IsChildBlock(std::vector<CodeLine>::iterator it, int previousLineLevel)
{
    return it->Level > previousLineLevel;
}

void HandleCompileMessage(int lineNumber)
{
    if(CompileMsgFlag)
    {
        CompileMsgPrint(lineNumber);
        CompileMsgFlag = false;
    }
}

void HandleDefineMethod(
    std::vector<CodeLine>::iterator* it,
    std::vector<CodeLine>::iterator* end,
    Scope* blockInheritedScope,
    Operation* op)
{
    // if(op->Type != OperationType::DefineMethod)
    //     return;

    // auto method = ObjectOf(op->Operands[0]->Value)->Action;

    // // assumes there is a block
    // int blockSize = SizeOfBlock((*it)+1, *end);
    // Block* b;
    // Scope* scope = ScopeConstructor(CurrentScope());
    // EnterScope(scope);
    // {
    //     for(auto param: method->ParameterNames)
    //     {
    //         NullReference(param);
    //     }
    //     b = ParseBlock((*it)+1, (*it)+blockSize+1);
    // }
    // ExitScope(true);

    // *it += blockSize;
    // ObjectOf(op->Operands.at(0)->Value)->Action->CodeBlock = b;
    // LogItDebug("exit method", "HandleDefineMethod");
}

Block* ParseBlock(
    std::vector<CodeLine>::iterator it, 
    std::vector<CodeLine>::iterator end,
    Scope* scope)
{
    Block* thisBlock = BlockConstructor();
    
    LogItDebug("entered new block", "ParseBlock");
    bool scopeIsLocal = false;

    if(scope == nullptr)
    {
        scope = ScopeConstructor(CurrentScope());
        scopeIsLocal = true;
    }

    EnterScope(scope);
    {
        int previousLineLevel = it->Level;

        for(; it != end; it++)
        {
            if(IsChildBlock(it, previousLineLevel))
            {
                LogItDebug(Msg("starting compile new block at line [%i]", it->LineNumber), "ParseBlock");
                int blockSize = SizeOfBlock(it, end);
                Block* b = ParseBlock(it, it+blockSize);
                if(FatalCompileError)
                    return nullptr;
                LogItDebug(Msg("finishes compile new block at line [%i]", it->LineNumber), "ParseBlock");

                // increment iterator to end of block
                it += blockSize - 1;

                thisBlock->Executables.push_back(b);
            }
            else
            {
                LogItDebug(Msg("starting compile line [%i]", it->LineNumber), "ParseBlock");
                Operation* op = ParseLine(it->Tokens);
                HandleCompileMessage(it->LineNumber);
                if(FatalCompileError)
                    return nullptr;

                NumberOperation(op, it->LineNumber);
                LogItDebug(Msg("finishes compile line [%i]", it->LineNumber), "ParseBlock");

                HandleDefineMethod(&it, &end, CurrentScope(), op);



                thisBlock->Executables.push_back(op);
            }
        }
    }
    ExitScope(scopeIsLocal);

    return thisBlock;
}

Program* ParseProgram(const std::string filepath)
{
    Program* p = ProgramConstructor();
    EnterProgram(p);

    std::fstream file;
    file.open(filepath, std::ios::in);
    if(!file.is_open())
    {
        std::cout << "\ncould not open file: " << filepath << std::endl;
    }

    int lineLevel = 0;
    int nextLinePos = 1;
    int lineStart = 1;

    for(std::string line = GetEffectiveLine(file, nextLinePos, lineStart); line != ""; line = GetEffectiveLine(file, nextLinePos, lineStart))
    {
        TokenList tokens = LexLine(line);
        lineLevel = LevelOfLine(line);

        CodeLine ls = { tokens , lineStart, lineLevel };
        p->Lines.push_back(ls);

    }

    // TODO: Allow different blocks
    Block* b = ParseBlock(p->Lines.begin(), p->Lines.end(), p->GlobalScope);
    p->Main = b;

    ExitProgram();
    return p;
}

void DeleteBlockRecursive(Block* b)
{
    for(auto exec: b->Executables)
    {
        if(exec->ExecType == ExecutableType::Block)
            DeleteBlockRecursive(static_cast<Block*>(exec));
        else
            DeleteOperationRecursive(static_cast<Operation*>(exec));
    }
    BlockDestructor(b);
}


void ProgramDestructor(Program* p)
{
    for(size_t i=0; i<p->ObjectsIndex.size(); i++)
    {
        auto& map = p->ObjectsIndex[i];
        for(auto ref: map.References)
        {
            ReferenceDestructor(ref);
        }
        if(map.IndexedObject == NullObject())
            continue;
        ObjectDestructor(map.IndexedObject);
    }

    ScopeDestructor(p->GlobalScope);

    for(auto codeLine: p->Lines)
    {
        DeleteTokenList(codeLine.Tokens);
    }

    DeleteBlockRecursive(p->Main);

    delete p;
}