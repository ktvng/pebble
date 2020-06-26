#ifndef __OPERATION_H
#define __OPERATION_H

#include "main.h"
#include "arch.h"
#include "token.h"
#include "object.h"
#include "diagnostics.h"


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

Reference* OperationAssign(Reference* lRef, Reference* rRef);
Reference* OperationPrint(const Reference* ref);
Reference* OperationAdd(const Reference* lRef, const Reference* rRef);
Reference* OperationSubtract(const Reference* lRef, const Reference* rRef);
Reference* OperationMultiply(const Reference* lRef, const Reference* rRef);
Reference* OperationDivide(const Reference* lRef, const Reference* rRef);
Reference* OperationAnd(const Reference* lRef, const Reference* rRef);
Reference* OperationDefine(Reference* ref);
Reference* OperationIf(Reference* ref);
Reference* OperationRef(Reference* ref);
Reference* OperationDefineMethod(Reference* ref);
Reference* OperationEvaluate(Reference* ref, std::vector<Reference*>& parameters);
Reference* OperationReturn(Reference* returnRef);
Reference* OperationTuple(const std::vector<Reference*>& components);

void DecideValueRef(TokenList& tokens, Reference** refValue);


// ---------------------------------------------------------------------------------------------------------------------
// Decide the probability that a given line [TokenList] is a particular atomic operation

void DecideProbabilityAdd(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityDefine(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityPrint(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityAssign(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityRef(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityIsEqual(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityIsLessThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityIsGreaterThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilitySubtract(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityMultiply(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityDivide(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityAnd(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityOr(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityNot(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityEvaluate(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityDefineMethod(PossibleOperationsList& typeProbabilities, const TokenList& tokens);


// ---------------------------------------------------------------------------------------------------------------------
// Decide the operands of an atomic operation.
//   should edit the [tokens] to remove used Tokens

void DecideOperandsAdd(TokenList& tokens, OperationsList& operands);
void DecideOperandsDefine(TokenList& tokens, OperationsList& operands);
void DecideOperandsPrint(TokenList& tokens, OperationsList& operands);
void DecideOperandsAssign(TokenList& tokens, OperationsList& operands);
void DecideOperandsIsEqual(TokenList& tokens, OperationsList& operands);
void DecideOperandsIsLessThan(TokenList& tokens, OperationsList& operands);
void DecideOperandsIsGreaterThan(TokenList& tokens, OperationsList& operands);
void DecideOperandsSubtract(TokenList& tokens, OperationsList& operands);
void DecideOperandsMultiply(TokenList& tokens, OperationsList& operands);
void DecideOperandsDivide(TokenList& tokens, OperationsList& operands);
void DecideOperandsAnd(TokenList& tokens, OperationsList& operands);
void DecideOperandsOr(TokenList& tokens, OperationsList& operands);
void DecideOperandsNot(TokenList& tokens, OperationsList& operands);
void DecideOperandsEvaluate(TokenList& tokens, OperationsList& operands);
void DecideOperandsRef(TokenList& tokens, OperationsList& operands);
void DecideOperandsDefineMethod(TokenList& tokens, OperationsList& operands);

#endif