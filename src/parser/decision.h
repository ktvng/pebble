#ifndef __DECISION_H
#define __DECISION_H

#include <algorithm>

#include "abstract.h"

// ---------------------------------------------------------------------------------------------------------------------
// Struct definitions

/// holds information on the probability that a given line is some operation with
/// [Type] as the OperationType
/// [Probability] the liklihood of this [Type]
struct OperationTypeProbability
{
    OperationType Type;
    double Probability;
};

/// possibly deprecated
enum LineType
{
    IfLine,
    WhileLine,
    Composite,
    Atomic,
};


typedef std::vector<OperationTypeProbability> PossibleOperationsList;


// ---------------------------------------------------------------------------------------------------------------------
// Deciding stuff
void DecideOperationTypeProbabilities(PossibleOperationsList& typeProbabilities, const TokenList& tokens);
void DecideLineType(PossibleOperationsList& typeProbabilities, const TokenList& tokens, LineType& lineType);
void DecideOperationType(PossibleOperationsList& typeProbabilities, OperationType& opType);
void DecideOperands(const OperationType& opType, TokenList& tokens, OperationsList& operands);


// ---------------------------------------------------------------------------------------------------------------------
// Decide the value of a ref operation

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
