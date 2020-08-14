#ifndef __OPERATION_H
#define __OPERATION_H

#include "abstract.h"


// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions







// ---------------------------------------------------------------------------------------------------------------------
// Constructors

Operation* OperationConstructor(
    OperationType type, 
    Reference* value = nullptr,
    OperationsList operands = {}
);

Operation* OperationConstructor(
    OperationType type, 
    OperationsList operands = {},
    Reference* value = nullptr
);

void DeleteOperationRecursive(Operation* op);
void OperationDestructor(Operation* op);

#endif
