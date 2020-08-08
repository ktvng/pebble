#ifndef __FLATTENER_H
#define __FLATTENER_H


#include "abstract.h"

struct JumpContext
{
    std::vector<extArg_t> UnresolvedJumps;
    extArg_t UnresolvedJumpFalse;
    bool HasUnresolvedJumpFalse;
};


// ---------------------------------------------------------------------------------------------------------------------
// Contants
inline constexpr extArg_t GOD_CALL_ID = 0;
inline constexpr extArg_t SOMETHING_CALL_ID = 1;
inline constexpr extArg_t NOTHING_CALL_ID = 2;
inline constexpr extArg_t ARRAY_CALL_ID = 3;
inline constexpr extArg_t PRIMITIVE_CALLS = 4;


extern int UniversalPrimitiveCount;

void FlattenProgram(Program* p);
void FlattenBlock(Block* block);
void FlattenOperation(Operation* op);

int NOPSafetyDomainSize();

void FlattenOperationScopeResolution(Operation* op);
void FlattenOperationRefDirect(Operation* op);

#endif