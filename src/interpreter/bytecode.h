#ifndef __BYTECODE_H
#define __BYTECODE_H

#include "abstract.h"


const uint8_t BitFlag = 0x1;


typedef void (*BCI_Method)(extArg_t);

const inline int BCI_NumberOfInstructions = 25;

void BCI_LoadRefName(extArg_t arg);
void BCI_LoadPrimitive(extArg_t arg);
void BCI_Dereference(extArg_t arg);

void BCI_Assign(extArg_t arg);

void BCI_Add(extArg_t arg);
void BCI_Subtract(extArg_t arg);
void BCI_Multiply(extArg_t arg);
void BCI_Divide(extArg_t arg);

void BCI_SysCall(extArg_t arg);

void BCI_And(extArg_t arg);
void BCI_Or(extArg_t arg);
void BCI_Not(extArg_t arg);

void BCI_Cmp(extArg_t arg);
void BCI_LoadCmp(extArg_t arg);

void BCI_JumpFalse(extArg_t arg);
void BCI_Jump(extArg_t arg);

void BCI_Copy(extArg_t arg);

void BCI_ResolveDirect(extArg_t arg);
void BCI_ResolveScoped(extArg_t arg);

void BCI_Eval(extArg_t arg);
void BCI_Return(extArg_t arg);

void BCI_EnterLocal(extArg_t arg);
void BCI_LeaveLocal(extArg_t arg);

void BCI_Extend(extArg_t arg);
void BCI_NOP(extArg_t arg);

extern BCI_Method BCI_Instructions[];

int IndexOfInstruction(BCI_Method bci);

#endif