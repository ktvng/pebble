#include <iostream>

#include "operation.h"
#include "reference.h"
#include "diagnostics.h"
#include "executable.h"
#include "object.h"


// ---------------------------------------------------------------------------------------------------------------------
// TODO:
// 1. Refactor code to put similar methods in the same section (i.e. DecideProbabilityX, OperationX, DecideOperandsX)
// 2. Implement unimplemented operations
// 3. Revamp the DecideOperandsX system


// ---------------------------------------------------------------------------------------------------------------------
// Creating operations

/// constructor for an operation of [type], with [operands], and value [value]. only operations
/// of type Ref::Operation should have non-nullptr value.
Operation* OperationConstructor(
    OperationType type, 
    OperationsList operands, 
    Reference* value)
{
    Operation* op = new Operation;
    op->LineNumber = -1;
    op->Operands = operands;
    op->Type = type;
    op->Value = value;

    op->ExecType = ExecutableType::Operation;

    return op;
}

/// constructor for an operation of [type], with [operands], and value [value]. only operations
/// of type Ref::Operation should have non-nullptr value. For memcheck purposes, cannot call the
/// other constructor
Operation* OperationConstructor(
    OperationType type, 
    Reference* value,
    OperationsList operands)
{
    Operation* op = new Operation;
    op->LineNumber = -1;
    op->Operands = operands;
    op->Type = type;
    op->Value = value;

    op->ExecType = ExecutableType::Operation;

    return op;
}

void OperationDestructor(Operation* op)
{
    delete op;
}

void DeleteOperationRecursive(Operation* op)
{
    for(auto operand: op->Operands)
        DeleteOperationRecursive(operand);
    
    if(op->Type == OperationType::Ref)
    {
        if(IsReferenceStub(op->Value))
        {
            ReferenceStubDestructor(op->Value);
        }
        else
        {
            ReferenceDestructor(op->Value);
        }
    }
    OperationDestructor(op);
}
