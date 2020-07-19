#ifndef __FLATTENER_H
#define __FLATTENER_H


#include "abstract.h"

struct JumpContext
{
    std::vector<extArg_t> UnresolvedJumps;
    std::vector<extArg_t> UnresolvedJumpFalses;
};

void FlattenProgram(Program* p);
void FlattenBlock(Block* block);
void FlattenOperation(Operation* op);

int NOPSafetyDomainSize();

void FlattenOperationScopeResolution(Operation* op);
void FlattenOperationScopeResolutionWithDereference(Operation* op);


#endif