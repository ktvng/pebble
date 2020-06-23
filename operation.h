#ifndef __OPERATION_H
#define __OPERATION_H

#include "main.h"
#include "arch.h"
#include "token.h"
#include "object.h"
#include "diagnostics.h"

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

Reference* OperationAssign(Reference* lRef, Reference* rRef);
Reference* OperationPrint(const Reference* ref);
Reference* OperationAdd(const Reference* lRef, const Reference* rRef);
Reference* OperationSubtract(const Reference* lRef, const Reference* rRef);
Reference* OperationMultiply(const Reference* lRef, const Reference* rRef);
Reference* OperationDivide(const Reference* lRef, const Reference* rRef);
Reference* OperationAnd(const Reference* lRef, const Reference* rRef);
Reference* OperationDefine(Reference* ref);
Reference* OperationIf(Reference* ref);
Reference* OperationReturn(Reference* ref);
Reference* OperationDefineMethod(Reference* ref);
Reference* OperationEvaluate(Reference* ref, std::vector<Reference*> parameters);


void DecideValueDefine(TokenList& tokens, Reference** value);
void DecideValueAssign(TokenList& tokens, Reference** refValue);
void DecideValueIsEqual(TokenList& tokens, Reference** refValue);
void DecideValueIsLessThan(TokenList& tokens, Reference** refValue);
void DecideValueIsGreaterThan(TokenList& tokens, Reference** refValue);
void DecideValueAdd(TokenList& tokens, Reference** refValue);
void DecideValueSubtract(TokenList& tokens, Reference** refValue);
void DecideValueMultiply(TokenList& tokens, Reference** refValue);
void DecideValueDivide(TokenList& tokens, Reference** refValue);
void DecideValueAnd(TokenList& tokens, Reference** refValue);
void DecideValueOr(TokenList& tokens, Reference** refValue);
void DecideValueNot(TokenList& tokens, Reference** refValue);
void DecideValueEvaluate(TokenList& tokens, Reference** refValue);
void DecideValuePrint(TokenList& tokens, Reference** refValue);
void DecideValueReturn(TokenList& tokens, Reference** refValue);
void DecideValueDefineMethod(TokenList& tokens, Reference** refValue);

// Decide Probabilities
void DecideProbabilityAdd(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityDefine(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityPrint(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityAssign(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideProbabilityReturn(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
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


// Decide Operands
// should edit token list remove used tokens
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
void DecideOperandsReturn(TokenList& tokens, OperationsList& operands);
void DecideOperandsDefineMethod(TokenList& tokens, OperationsList& operands);

#endif