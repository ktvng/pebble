#ifndef __OPERATION_H
#define __OPERATION_H

#include "main.h"
#include "token.h"
#include "object.h"



// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// different atomic operations which build Pebble code
enum OperationType
{
    Assign,                         // change what a Reference points to
    IsEqual,                        // returns Reference to BooleanClass result of ==
    IsNotEqual,                     // returns Reference to BooleanClass result of !=
    IsLessThan,                     // returns Reference to BooleanClass result of <
    IsGreaterThan,                  // returns Reference to BooleanClass result of >
    IsLessThanOrEqualTo,            // returns Reference to BooleanClass result of <=
    IsGreaterThanOrEqualTo,         // returns Reference to BooleanClass result of >=

    Add,                            // returns Reference to result of +
    Subtract,                       // returns Reference to result of -
    Multiply,                       // returns Reference to result of *
    Divide,                         // returns Reference to result of /

    And,                            // returns Reference to result of &&
    Or,                             // returns Reference to result of ||
    Not,                            // returns Reference to result of !
    Evaluate,                       // returns Reference to result of method call
    Print,                          // prints a Ref to the screen
    
    Ref,                            // terminal of an operation tree, returns a Reference
    DefineMethod,                   // add a new method Reference to scope, but returns NullReference
    Return,                         // break out of a method and return a value

    If,                             // conditionally executes the next block of code
    While,
    EndLabel,                       // end of an if statement
    Tuple,                          // constructs and returns a (>1) ordering of references
    New,
    ScopeResolution,
    Class,


};

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

Method* MethodConstructor(Scope* inheritedScope);


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
