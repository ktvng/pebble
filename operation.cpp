#include <iostream>

#include "operation.h"
#include "program.h"
#include "object.h"

// ---------------------------------------------------------------------------------------------------------------------
// Creating operations
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

Operation* OperationConstructor(
    OperationType type, 
    Reference* value,
    OperationsList operands)
{
    return OperationConstructor(type, operands, value);
}





/// adds a new return Operation for [ref] to [operands]
void AddReferenceReturnOperationTo(OperationsList& operands, Reference* ref)
{
    operands.push_back(OperationConstructor(OperationType::Return, ref));
}

/// adds an existing Operation [op] to [operands]
void AddOperationTo(OperationsList& operands, Operation* op)
{
    operands.push_back(op);
}









// ---------------------------------------------------------------------------------------------------------------------
// Atomic Operations

Reference* OperationReturn(Reference* ref, Scope* scope)
{
    return DefineNewReference(c_returnReferenceName, ref->ToObject, scope);
}

Reference* OperationAssign(Reference* lRef, Reference* rRef, Scope* scope)
{
    ReassignReference(lRef, rRef->ToObject);
    return DefineNewReference(c_returnReferenceName, lRef->ToObject, scope);
}

Reference* OperationPrint(const Reference* ref, Scope* scope)
{
    std::cout << GetStringValue(*ref->ToObject) << "\n";
    return DefineNewReference(c_returnReferenceName, ref->ToObject, scope);
}

Reference* OperationAdd(const Reference* lRef, const Reference* rRef, Scope* scope)
{
    Reference* resultRef;

    if(IsNumeric(*lRef) && IsNumeric(*rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*lRef->ToObject, *rRef->ToObject);
        if(type == IntegerClass)
        {
            int value = GetIntValue(*lRef->ToObject) + GetIntValue(*rRef->ToObject);
            resultRef = DefineNewReference(c_returnReferenceName, value, scope);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*lRef->ToObject) + GetDecimalValue(*rRef->ToObject);
            resultRef = DefineNewReference(c_returnReferenceName, value, scope);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Add", "unimplemented");
            resultRef = DefineNullReference(scope);
        }
        return resultRef;
    }

    resultRef = DefineNullReference(scope);
    ReportRuntimeMsg(SystemMessageType::Warning, MSG("cannot add types %s and %s", lRef->ToObject->Class, rRef->ToObject->Class));
    return resultRef;
}

Reference* OperationAnd(const Reference* lRef, const Reference* rRef, Scope* scope)
{
    bool b = GetBoolValue(*lRef->ToObject) && GetBoolValue(*rRef->ToObject);
    return DefineNewReference(c_returnReferenceName, b, scope);
}

Reference* OperationDefine(Reference* ref, Scope* scope)
{
    LogItDebug(MSG("added reference [%s] to scope", ref->Name), "OperationDefine");
    AddReferenceToScope(ref, scope);

    Reference* returnRef = DefineNewReference(c_returnReferenceName, ref->ToObject, scope);
    return returnRef;
}

Reference* OperationSubtract(const Reference* lRef, const Reference* rRef, Scope* scope)
{
    Reference* resultRef;

    if(IsNumeric(*lRef) && IsNumeric(*rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*lRef->ToObject, *rRef->ToObject);
        if(type == IntegerClass)
        {
            int value = GetIntValue(*lRef->ToObject) - GetIntValue(*rRef->ToObject);
                resultRef = DefineNewReference(c_returnReferenceName, value, scope);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*lRef->ToObject) - GetDecimalValue(*rRef->ToObject);
                resultRef = DefineNewReference(c_returnReferenceName, value, scope);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Subtract", "unimplemented");
            resultRef = DefineNullReference(scope);
        }
        return resultRef;
    }

    resultRef = DefineNullReference(scope);
    ReportRuntimeMsg(SystemMessageType::Warning, MSG("cannot subtract %s from %s", rRef->ToObject->Class, lRef->ToObject->Class));
    return resultRef;
}

Reference* OperationIf(Reference* ref, Scope* scope)
{
    return DefineNewReference(c_returnReferenceName, ref->ToObject, scope);
}




// ---------------------------------------------------------------------------------------------------------------------
// Decide Probabilities

void DecideProbabilityAdd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> addKeyWords = { "add", "plus", "+", "adding" };
    if(TokenListContainsContent(tokens, addKeyWords))
    {
        OperationTypeProbability addType = { OperationType::Add, 4.0 };
        typeProbabilities.push_back(addType);
    }
}

void DecideProbabilityDefine(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> defineKeyWords = { "define", "let", "make", "declare" };
    if(TokenListContainsContent(tokens, defineKeyWords))
    {
        OperationTypeProbability defineType = { OperationType::Define, 4.0};
        typeProbabilities.push_back(defineType);
    }
}

void DecideProbabilityPrint(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> printKeyWords = { "print", "display", "show", "output" };
    if(TokenListContainsContent(tokens, printKeyWords))
    {
        OperationTypeProbability printType = { OperationType::Print, 4.0 };
        typeProbabilities.push_back(printType);
    }
}

void DecideProbabilityAssign(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(Token* pos = FindToken(tokens, "="); pos != nullptr)
    {
        OperationTypeProbability assignType = { OperationType::Assign, 10 };
        typeProbabilities.push_back(assignType);
    }
}

void DecideProbabilityReturn(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    OperationTypeProbability returnType = { OperationType::Return, 1 };
    typeProbabilities.push_back(returnType);
}

void DecideProbabilityIsEqual(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{

}

void DecideProbabilityIsLessThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityIsGreaterThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilitySubtract(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> subKeyWords = { "sub", "subtract", "minus", "-", "subtracting" };
    if(TokenListContainsContent(tokens, subKeyWords))
    {
        OperationTypeProbability subType = { OperationType::Subtract, 4.0 };
        typeProbabilities.push_back(subType);
    }
}

void DecideProbabilityMultiply(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityDivide(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityAnd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> andKeyWords = { "and", "&&", "together", "with" };
    if(TokenListContainsContent(tokens, andKeyWords))
    {
        OperationTypeProbability andType = { OperationType::And, 4.0 };
        typeProbabilities.push_back(andType);
    }
}

void DecideProbabilityOr(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityNot(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityEvaluate(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}







// ---------------------------------------------------------------------------------------------------------------------
// Decide operation Values

void UnimplementedValueFunction(const OperationType opType, Reference** refValue)
{
    *refValue = nullptr;
    LogIt(LogSeverityType::Sev1_Notify, "DecideOperationValue", MSG("unimplemented in case: %s", ToString(opType)));
}


void DecideValueDefine(Scope* scope, TokenList& tokens, Reference** refValue)
{
    Reference* ref; 

    Token* name = NextTokenMatching(tokens, TokenType::Reference);
    Token* value = NextTokenMatching(tokens, PrimitiveTokenTypes);

    if(name == nullptr){
        LogIt(LogSeverityType::Sev3_Critical, "DecideOperandsDefine", "cannot determine reference name");
        ReportCompileMsg(SystemMessageType::Exception, "cannot determine reference name");
        // TODO: should be critical error
        *refValue = DefineNullReference(scope); 
        return;
    }
    
    if(value == nullptr)
    {
        ReportCompileMsg(SystemMessageType::Exception, "cannot determine reference value");
        ref = DefineNullReference(name->Content, scope);
    }
    else
    {
        ref = DefineNewReference(name->Content, value, scope);
    }
    *refValue = ref;
}

void DecideValueAssign(Scope* scope, TokenList& tokens, Reference** refValue)
{
    int pos = 0;
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, TokenType::Reference, pos), scope);

    TokenList rightTokens = RightOfToken(tokens, tokens.at(pos));
    tokens = rightTokens;

    *refValue = arg1;
}

void DecideValueIsEqual(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::IsEqual, refValue);
}

void DecideValueIsLessThan(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::IsLessThan, refValue);
}

void DecideValueIsGreaterThan(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::IsGreaterThan, refValue);
}

void DecideValueAdd(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Add, refValue);
}

void DecideValueSubtract(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Subtract, refValue);
}

void DecideValueMultiply(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Multiply, refValue);
}

void DecideValueDivide(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Divide, refValue);
}

void DecideValueAnd(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::And, refValue);
}

void DecideValueOr(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Or, refValue);
}

void DecideValueNot(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Not, refValue);
}

void DecideValueEvaluate(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Evaluate, refValue);
}

void DecideValuePrint(Scope* scope, TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Print, refValue);
}

void DecideValueReturn(Scope* scope, TokenList& tokens, Reference** refValue)
{
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes), scope);
    *refValue = arg1;
}









// ---------------------------------------------------------------------------------------------------------------------
// Decide Operands
// should edit token list remove used tokens

void GetTwoOperands(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    int pos = 0;
 
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes, pos), scope, c_operationReferenceName);
    Reference* arg2 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes, pos), scope, c_operationReferenceName);

    AddReferenceReturnOperationTo(operands, arg1);
    AddReferenceReturnOperationTo(operands, arg2);
}


void DecideOperandsAdd(Scope* scope, TokenList& tokens, OperationsList& operands) // EDIT
{
    GetTwoOperands(scope, tokens, operands);
}

void DecideOperandsDefine(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    // no operands
}

void DecideOperandsPrint(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes), scope, c_operationReferenceName);
    AddReferenceReturnOperationTo(operands, arg1);
}

void DecideOperandsAssign(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    Operation* op2 = ParseLine(scope, tokens);
    AddOperationTo(operands, op2);
}

void DecideOperandsIsEqual(Scope* scope, TokenList& tokens, OperationsList& operands)
{

}

void DecideOperandsIsLessThan(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsIsGreaterThan(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsSubtract(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    GetTwoOperands(scope, tokens, operands);
}

void DecideOperandsMultiply(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsDivide(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsAnd(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    GetTwoOperands(scope, tokens, operands);
}

void DecideOperandsOr(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsNot(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsEvaluate(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsReturn(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    // no operands
}