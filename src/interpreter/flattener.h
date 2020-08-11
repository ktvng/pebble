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
constexpr extArg_t GOD_CALL_ID = 0;
constexpr extArg_t SOMETHING_CALL_ID = 1;
constexpr extArg_t NOTHING_CALL_ID = 2;
constexpr extArg_t ARRAY_CALL_ID = 3;
constexpr extArg_t INTEGER_CALL_ID = 4;
constexpr extArg_t DECIMAL_CALL_ID = 5;
constexpr extArg_t STRING_CALL_ID = 6;
constexpr extArg_t BOOLEAN_CALL_ID = 7;
constexpr extArg_t PRIMITIVE_CALLS = 8;


extern int UniversalPrimitiveCount;

void FlattenProgram(Program* p);
void FlattenBlock(Block* block);
void FlattenOperation(Operation* op);

int NOPSafetyDomainSize();

void FlattenOperationScopeResolution(Operation* op);
void FlattenOperationRefDirect(Operation* op);

#endif