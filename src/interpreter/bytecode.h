#ifndef __BYTECODE_H
#define __BYTECODE_H

#include "abstract.h"

// ---------------------------------------------------------------------------------------------------------------------
// Constants

/// unsigned int representing the the first bit in a byte
constexpr uint8_t BitFlag = 0x1;

/// number of bytecode instructions 
constexpr int BCI_NumberOfInstructions = 36;

// ---------------------------------------------------------------------------------------------------------------------
// Bytecode instructions methods

/// function type of a bytecode instruction
typedef void (*BCI_Method)(extArg_t);


void BCI_LoadCallName(extArg_t arg);
void BCI_LoadPrimitive(extArg_t arg);
void BCI_Assign(extArg_t arg);
void BCI_Add(extArg_t arg);
void BCI_Subtract(extArg_t arg);

void BCI_Multiply(extArg_t arg);
void BCI_Divide(extArg_t arg);
void BCI_SysCall(extArg_t arg);
void BCI_And(extArg_t arg);
void BCI_Or(extArg_t arg);

void BCI_Not(extArg_t arg);
void BCI_NotEquals(extArg_t arg);
void BCI_Equals(extArg_t arg);
void BCI_Cmp(extArg_t arg);
void BCI_LoadCmp(extArg_t arg);

void BCI_JumpFalse(extArg_t arg);
void BCI_Jump(extArg_t arg);
void BCI_Copy(extArg_t arg);
void BCI_BindType(extArg_t arg);
void BCI_ResolveDirect(extArg_t arg);

void BCI_ResolveScoped(extArg_t arg);
void BCI_BindScope(extArg_t arg);
void BCI_BindSection(extArg_t arg);
void BCI_EvalHere(extArg_t arg);
void BCI_Eval(extArg_t arg);

void BCI_Return(extArg_t arg);
void BCI_Array(extArg_t arg);
void BCI_EnterLocal(extArg_t arg);
void BCI_LeaveLocal(extArg_t arg);
void BCI_Extend(extArg_t arg);

void BCI_NOP(extArg_t arg);
void BCI_Dup(extArg_t arg);
void BCI_EndLine(extArg_t arg);
void BCI_Swap(extArg_t arg);
void BCI_JumpNothing(extArg_t arg);

void BCI_DropTOS(extArg_t arg);

// ---------------------------------------------------------------------------------------------------------------------
// Bytecode instructions

/// array of all recognized bytecode instructions
extern BCI_Method BCI_Instructions[];


// ---------------------------------------------------------------------------------------------------------------------
// Helper methods

/// returns the index of [bci] in BCI_Instructions
int IndexOfInstruction(BCI_Method bci);

/// true if the name of call is a keyword
inline bool CallNameIsKeyword(const String* name)
{
    return *name == "caller" || *name == "self" || *name == "it" || *name == "that";
}

/// true if call is bound to NothingType or if it is bound to NothingScope
bool IsNothing(const Call* call);

#endif
