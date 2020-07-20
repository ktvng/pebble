#ifndef __EXECUTE_H
#define __EXECUTE_H

#include "abstract.h"


typedef Reference* (*OperationEvaluator)(Reference*, std::vector<Reference*>&);
extern OperationEvaluator OperationEvaluators[];

Reference* DoOperation(Operation* op);
Reference* DoBlock(Block* codeBlock, Scope* scope=nullptr);
void DoProgram(Program* program);

#endif
