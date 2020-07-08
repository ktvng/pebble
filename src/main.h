#ifndef __MAIN_H
#define __MAIN_H

#include <string>
#include <vector>
#include <map>

#include "utils.h"

// ---------------------------------------------------------------------------------------------------------------------
// Declarations

struct Token;
struct OperationTypeProbability;
struct SystemMessage;
struct ObjectReferenceMap;
struct Scope;
struct CodeLine;
struct Program;
struct Reference;


// ---------------------------------------------------------------------------------------------------------------------
// Executables

/// type of executable, either a single line (Operation), or list of lines (Block)
enum class ExecutableType
{
    Operation,
    Block
};

/// abstract class for anything build from operations
class Executable 
{
    public:
    ExecutableType ExecType;
};



/// a sequential list of atomic operations/Blocks is a block
/// [Executables] is the list child Execuable objects
class Block : public Executable
{
    public:
    std::vector<Executable*> Executables;
};


// ---------------------------------------------------------------------------------------------------------------------
// Typedefs

typedef std::string String;


/// if true will turn on output for 'print' command of the language
extern bool g_outputOn;

#endif
