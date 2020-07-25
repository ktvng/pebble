// #ifndef __RUNTIME_H
// #define __RUNTIME_H

// #include "abstract.h"

// typedef std::vector<Operation*> OperationsList;


// // ---------------------------------------------------------------------------------------------------------------------
// // Handle the execution of atomic operations

// Reference* OperationDefine(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationAssign(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationIsEqual(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationIsNotEqual(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationIsLessThan(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationIsGreaterThan(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationIsLessThanOrEqualTo(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationIsGreaterThanOrEqualTo(Reference* value, std::vector<Reference*>& operands);


// Reference* OperationAdd(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationSubtract(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationMultiply(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationDivide(Reference* value, std::vector<Reference*>& operands);

// Reference* OperationAnd(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationOr(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationNot(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationEvaluate(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationPrint(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationAsk(Reference* value, std::vector<Reference*>& operands);

// Reference* OperationRef(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationDefineMethod(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationReturn(Reference* value, std::vector<Reference*>& operands);

// Reference* OperationIf(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationElseIf(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationElse(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationWhile(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationEndLabel(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationTuple(Reference* value, std::vector<Reference*>& operands);

// Reference* OperationNew(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationScopeResolution(Reference* value, std::vector<Reference*>& operands);
// Reference* OperationClass(Reference* value, std::vector<Reference*>& operands);

// #endif
