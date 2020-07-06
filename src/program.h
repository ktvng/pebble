#ifndef __PROGRAM_H
#define __PROGRAM_H

#include "main.h"
#include "diagnostics.h"
#include "execute.h"


// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// used to keep track of objects and their references
struct ObjectReferenceMap
{
    Object* Object;
    std::vector<Reference*> References;
};

struct MethodReferenceMap
{
    Method* Method;
    std::vector<Reference*> References;
};


/// represents the result of tokenizing a line with additional metadata
/// [Tokens] are the tokens resulting from processing an effective line of code
/// [LineNumber] is the line of code which was processed
/// [Level] is the indent level of the line
struct CodeLine
{
    TokenList Tokens;
    int LineNumber;
    int Level;
};

/// a structure which contains a runnable representation of Pebble code
/// [Lines] are the 'raw' tokenized CodeLines of the program
/// [GlobalScope] (possible deprecated) is the global scope of the program
/// [ObjectsIndex] contains all objects and their references during program execution
struct Program
{
    std::vector<CodeLine> Lines;
    Scope* GlobalScope;
    Block* Main;
    std::vector<ObjectReferenceMap*> ObjectsIndex;
    std::vector<MethodReferenceMap*> MethodsIndex;
    Reference* That;
};


/// log all messages at or above this level
extern LogSeverityType LogAtLevel;

extern bool override;

const bool c_DEBUG = true;
const bool c_ERROR = true;

extern bool FatalCompileError;

extern bool RuntimeMsgFlag;
extern bool CompileMsgFlag;

extern std::vector<SystemMessage> RuntimeMsgBuffer;
extern std::vector<SystemMessage> CompileMsgBuffer;

extern int RuntimeMsgCount;
extern int CompileMsgCount;

extern std::string ProgramOutput;
extern std::string ProgramMsgs;

extern Program* PROGRAM;


// ---------------------------------------------------------------------------------------------------------------------
// Method declarations

Block* BlockConstructor();

Operation* ParseLine(TokenList& tokens);
Block* ParseBlock(std::vector<CodeLine>::iterator it, std::vector<CodeLine>::iterator end, Scope* scope=nullptr);
Program* ParseProgram(const std::string filepath);

void ReportCompileMsg(SystemMessageType type, String message);
void ReportRuntimeMsg(SystemMessageType type, String message);

#endif
