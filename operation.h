#ifndef __OPERATION_H
#define __OPERATION_H

#include "main.h"
#include "arch.h"
#include "token.h"
#include "object.h"
#include "diagnostics.h"


/** /
typedef Reference* (*OperationFunction)(OperationInfo*);
OperationFunction operations[] = 
{
    OperationDefine,
    OperationAssign,
    OperationIsEqual,
    OperationIsLessThan,
    OperationIsGreaterThan,
    OperationAdd, 
    OperationSubtract,
    OperationMultiply,
    OperationDivide,
    OperationAnd, 
    OperationOr,
    OperationNot,
    OperationEvaluate,
    OperationPrint,
    OperationReturn, 
};
/** / 

struct OperationInfo
{
    Reference *mainRef;
    Reference *secondRef;
    TokenList *tokens;
    Scope *scope;
};
/**/
void AddReferenceToScope(Reference* ref, Scope* scope);
Operation* CreateOperation();

Reference* OperationAssign(Reference* lRef, Reference* rRef);
Reference* OperationPrint(const Reference* ref);
Reference* OperationAdd(const Reference* lRef, const Reference* rRef);
Reference* OperationSubtract(const Reference* lRef, const Reference* rRef);
Reference* OperationAnd(const Reference* lRef, const Reference* rRef);
Reference* OperationDefine(Reference* ref, Scope* scope);



void DecideValueDefine(Scope* scope, TokenList& tokens, Reference** value);
void DecideValueAssign(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueIsEqual(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueIsLessThan(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueIsGreaterThan(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueAdd(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueSubtract(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueMultiply(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueDivide(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueAnd(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueOr(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueNot(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueEvaluate(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValuePrint(Scope* scope, TokenList& tokens, Reference** refValue);
void DecideValueReturn(Scope* scope, TokenList& tokens, Reference** refValue);

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



// Decide Operands
// should edit token list remove used tokens
void DecideOperandsAdd(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsDefine(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsPrint(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsAssign(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsIsEqual(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsIsLessThan(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsIsGreaterThan(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsSubtract(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsMultiply(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsDivide(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsAnd(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsOr(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsNot(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsEvaluate(Scope* scope, TokenList& tokens, OperationsList& operands);
void DecideOperandsReturn(Scope* scope, TokenList& tokens, OperationsList& operands);

#endif