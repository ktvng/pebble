#ifndef __OPERATION_H
#define __OPERATION_H

#include "abstract.h"
#include "executable.h"


// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions



/// a representation of an atomic operation with
/// [Type] which governs how the operation should be executed
/// [Operands] which will be evaluated into the inputs to this operation
/// [Value] which is only used for OperationType::Ref (terminals) of the operation tree
/// [LineNumber] which is the line of code that the operation was parsed from
class Operation : public Executable 
{
    public:
    OperationType Type;
    std::vector<Operation*> Operands;
    Reference* Value;
    int LineNumber;

    /// used for bytecode
    int EntityIndex;
};

typedef std::vector<Operation*> OperationsList;



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

// ---------------------------------------------------------------------------------------------------------------------
// Handle the execution of atomic operations

Reference* OperationDefine(Reference* value, std::vector<Reference*>& operands);
Reference* OperationAssign(Reference* value, std::vector<Reference*>& operands);
Reference* OperationIsEqual(Reference* value, std::vector<Reference*>& operands);
Reference* OperationIsNotEqual(Reference* value, std::vector<Reference*>& operands);
Reference* OperationIsLessThan(Reference* value, std::vector<Reference*>& operands);
Reference* OperationIsGreaterThan(Reference* value, std::vector<Reference*>& operands);
Reference* OperationIsLessThanOrEqualTo(Reference* value, std::vector<Reference*>& operands);
Reference* OperationIsGreaterThanOrEqualTo(Reference* value, std::vector<Reference*>& operands);


Reference* OperationAdd(Reference* value, std::vector<Reference*>& operands);
Reference* OperationSubtract(Reference* value, std::vector<Reference*>& operands);
Reference* OperationMultiply(Reference* value, std::vector<Reference*>& operands);
Reference* OperationDivide(Reference* value, std::vector<Reference*>& operands);

Reference* OperationAnd(Reference* value, std::vector<Reference*>& operands);
Reference* OperationOr(Reference* value, std::vector<Reference*>& operands);
Reference* OperationNot(Reference* value, std::vector<Reference*>& operands);
Reference* OperationEvaluate(Reference* value, std::vector<Reference*>& operands);
Reference* OperationPrint(Reference* value, std::vector<Reference*>& operands);

Reference* OperationRef(Reference* value, std::vector<Reference*>& operands);
Reference* OperationDefineMethod(Reference* value, std::vector<Reference*>& operands);
Reference* OperationReturn(Reference* value, std::vector<Reference*>& operands);

Reference* OperationIf(Reference* value, std::vector<Reference*>& operands);
Reference* OperationWhile(Reference* value, std::vector<Reference*>& operands);
Reference* OperationEndLabel(Reference* value, std::vector<Reference*>& operands);
Reference* OperationTuple(Reference* value, std::vector<Reference*>& operands);

Reference* OperationNew(Reference* value, std::vector<Reference*>& operands);
Reference* OperationScopeResolution(Reference* value, std::vector<Reference*>& operands);
Reference* OperationClass(Reference* value, std::vector<Reference*>& operands);

#endif
