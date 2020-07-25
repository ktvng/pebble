#ifndef __EXECUTABLE_H
#define __EXECutABLE_H

#include "abstract.h"


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


/// a representation of an atomic operation with
/// [Type] which governs how the operation should be executed
/// [Operands] which will be evaluated into the inputs to this operation
/// [Value] which is only used for OperationType::Ref (terminals) of the operation tree
/// [LineNumber] which is the line of code that the operation was parsed from
class Operation : public Executable 
{
    public:
    OperationType Type;
    std::vector<Operation*> Operands;
    Reference* Value;
    int LineNumber;

    /// used for bytecode
    int EntityIndex;
};

/// a sequential list of atomic operations/Blocks is a block
/// [Executables] is the list child Execuable objects
class Block : public Executable
{
    public:
    std::vector<Executable*> Executables;
};

Block* BlockConstructor();
void BlockDestructor(Block* b);
void DeleteBlockRecursive(Block* b);

#endif
