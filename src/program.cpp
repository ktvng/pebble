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
#include "scope.h"
#include "executable.h"

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

Program* ProgramConstructor()
{
    Program* p = new Program;
    p->GlobalScope = ScopeConstructor(nullptr);
    p->GlobalScope->IsDurable = true;

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

TokenList RemoveComment(TokenList tokens)
{
    TokenList tokensNoComments;
    bool foundComment = false; 
    for (size_t i = 0; i < tokens.size(); i++)
    { 
        if (tokens.at(i)->Content != "#" && !foundComment) 
        {
            tokensNoComments.push_back(tokens.at(i));
        } 
        else if (tokens.at(i)->Content != "#" && foundComment)
        {
            TokenDestructor(tokens.at(i));
        } 
        else if (tokens.at(i)->Content == "#") 
        {
            foundComment = true;
            TokenDestructor(tokens.at(i));
        }
    }  
    return tokensNoComments;
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

bool LineIsComment(std::string &line)
{
    size_t i = 0;
    while (i < line.size())
    {
        if (!CharIsWhiteSpace(line.at(i)))
            break;
        i++;
    }    
    return line[i] == '#' ? true : false;
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

        if (LineIsComment(newLine))
            continue;

        if(FirstTime)
        {
            lineStart = lineNumber - 1;
            FirstTime = false;
        }
        fullLine += newLine;

    } while (LineIsWhitespace(newLine) || LineIsComment(newLine));

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
    return nullptr;
    /// EDITED
    // OperationType opType;
    // DecideOperationType(typeProbabilities, opType);

    // LogItDebug(Msg("operation type is %s", ToString(opType)), "ParseOutAtomic");
    // // these operands act on references!
    // Reference* refValue = nullptr;
    // if(opType == OperationType::Ref)
    // {
    //     DecideValueRef(tokens, &refValue);
    // }

    // OperationsList operands;
    // DecideOperands(opType, tokens, operands);

    // Operation* op = OperationConstructor(opType, operands, refValue);

    // return op;
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

/// Checks for errors programmers may make and given better suggestions (TODO: make message )
void CheckForLineErrors(TokenList* tokensPtr)
{
    for(size_t i = 0; i < tokensPtr->size() - 1; i++)
    {
        if(tokensPtr->at(i)->Type == TokenType::Simple && tokensPtr->at(i + 1)->Type == TokenType::Simple)
        {
            LogDiagnostics(tokensPtr, "Cannot have multiple operators next to each other");
            LogIt(LogSeverityType::Sev3_Critical, "ERROR", "Cannot have multiple operators next to each other");
        }
    }
    // determines if the first or last oken is an operator (with 1#'s being the exception at index 0)
    if((tokensPtr->at(0)->Type == TokenType::Simple && tokensPtr->at(0)->Content.find("-") != std::string::npos) || tokensPtr->at(tokensPtr->size() - 1)->Type == TokenType::Simple)
    {
        LogDiagnostics(tokensPtr, "Unfinished expression");
        LogIt(LogSeverityType::Sev3_Critical, "ERROR", "Unfinished expression");

    }
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

/// parses a line of tokens that make up 1 line of code
Operation* ParseLine(TokenList& tokens)
{
    return ExpressionParser(tokens);
    /// EDITED
    // PossibleOperationsList typeProbabilities;
    // DecideOperationTypeProbabilities(typeProbabilities, tokens);

    // LineType lineType;
    // DecideLineType(typeProbabilities, tokens, lineType);
    
    // // fPtr
    // switch(lineType)
    // {
    //     case LineType::Atomic:
    //     return ParseOutAtomic(typeProbabilities, tokens);

    //     case LineType::Composite:
    //     return ParseComposite(typeProbabilities, tokens);

    //     case LineType::IfLine:
    //     return ParseIf(typeProbabilities, tokens);

    //     case LineType::WhileLine:
    //     return ParseWhile(typeProbabilities, tokens);

    //     default:
    //     LogIt(LogSeverityType::Sev1_Notify, "ParseLine", "unimplemented in case");
    //     return nullptr;
    // }
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

Block* ParseBlock(
    std::vector<CodeLine>::iterator it, 
    std::vector<CodeLine>::iterator end,
    Scope* scope)
{
    Block* thisBlock = BlockConstructor();
    
    LogItDebug("entered new block", "ParseBlock");

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

            thisBlock->Executables.push_back(op);
        }
    }

    return thisBlock;
}

Program* ParseProgram(const std::string filepath)
{
    InitParser();
    
    Program* p = ProgramConstructor();

    std::fstream file;
    file.open(filepath, std::ios::in);
    if(!file.is_open())
    {
        std::cout << "\ncould not open file: " << filepath << std::endl;
        return nullptr;
    }

    int lineLevel = 0;
    int nextLinePos = 1;
    int lineStart = 1;

    // can reduce logic and simplify
    for(std::string line = GetEffectiveLine(file, nextLinePos, lineStart); line != ""; line = GetEffectiveLine(file, nextLinePos, lineStart))
    {
        // std::string removed = RemoveComment(line);
        TokenList tokens = LexLine(line);
        TokenList noComments = RemoveComment(tokens);
        lineLevel = LevelOfLine(line);

        CodeLine ls = { noComments , lineStart, lineLevel };
        p->Lines.push_back(ls);

    }

    // TODO: Allow different blocks
    Block* b = ParseBlock(p->Lines.begin(), p->Lines.end(), p->GlobalScope);
    p->Main = b;

    return p;
}

void ProgramDestructor(Program* p)
{
    ScopeDestructor(p->GlobalScope);

    for(auto codeLine: p->Lines)
    {
        DeleteTokenList(codeLine.Tokens);
    }

    DeleteBlockRecursive(p->Main);

    delete p;
}