#include "executable.h"

#include "operation.h"

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

void DeleteBlockRecursive(Block* b)
{
    for(auto exec: b->Executables)
    {
        if(exec->ExecType == ExecutableType::Block)
        {
            DeleteBlockRecursive(static_cast<Block*>(exec));
        }
        else
        {
            DeleteOperationRecursive(static_cast<Operation*>(exec));
        }
    }
    
    BlockDestructor(b);
}
