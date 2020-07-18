#ifndef __FLATTENER_H
#define __FLATTENER_H


#include "abstract.h"

void FlattenProgram(Program* p);
void FlattenBlock(Block* block);
void FlattenOperation(Operation* op);

#endif