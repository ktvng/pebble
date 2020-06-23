#include <iostream>

#include "operation.h"
#include "program.h"
#include "reference.h"
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

Reference* OperationReturn(Reference* ref)
{
    if(ref->ToObject != nullptr)
        return ReferenceFor(c_returnReferenceName, ref->ToObject);
    else if(ref->ToMethod != nullptr)
        return ReferenceFor(c_returnReferenceName, ref->ToMethod);
}

Reference* OperationAssign(Reference* lRef, Reference* rRef)
{
    ReassignReference(lRef, rRef->ToObject);
    return ReferenceFor(c_returnReferenceName, lRef->ToObject);
}

Reference* OperationPrint(const Reference* ref)
{
    std::cout << GetStringValue(*ref->ToObject) << "\n";
    return ReferenceFor(c_returnReferenceName, ref->ToObject);
}

Reference* OperationAdd(const Reference* lRef, const Reference* rRef)
{
    Reference* resultRef;

    if(IsNumeric(*lRef) && IsNumeric(*rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*lRef->ToObject, *rRef->ToObject);
        if(type == IntegerClass)
        {
            int value = GetIntValue(*lRef->ToObject) + GetIntValue(*rRef->ToObject);
            resultRef = ReferenceFor(c_returnReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*lRef->ToObject) + GetDecimalValue(*rRef->ToObject);
            resultRef = ReferenceFor(c_returnReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Add", "unimplemented");
            resultRef = NullReference();
        }
        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, MSG("cannot add types %s and %s", lRef->ToObject->Class, rRef->ToObject->Class));
    return resultRef;
}

Reference* OperationAnd(const Reference* lRef, const Reference* rRef)
{
    bool b = GetBoolValue(*lRef->ToObject) && GetBoolValue(*rRef->ToObject);
    return ReferenceFor(c_returnReferenceName, b);
}

Reference* OperationDefine(Reference* ref)
{
    LogItDebug(MSG("added reference [%s] to scope", ref->Name), "OperationDefine");
    AddReferenceToCurrentScope(ref);

    Reference* returnRef = ReferenceFor(c_returnReferenceName, ref->ToObject);
    return returnRef;
}

Reference* OperationSubtract(const Reference* lRef, const Reference* rRef)
{
    Reference* resultRef;

    if(IsNumeric(*lRef) && IsNumeric(*rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*lRef->ToObject, *rRef->ToObject);
        if(type == IntegerClass)
        {
            int value = GetIntValue(*lRef->ToObject) - GetIntValue(*rRef->ToObject);
                resultRef = ReferenceFor(c_returnReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*lRef->ToObject) - GetDecimalValue(*rRef->ToObject);
                resultRef = ReferenceFor(c_returnReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Subtract", "unimplemented");
            resultRef = NullReference();
        }
        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, MSG("cannot subtract %s from %s", rRef->ToObject->Class, lRef->ToObject->Class));
    return resultRef;
}

Reference* OperationIf(Reference* ref)
{
    return ReferenceFor(c_returnReferenceName, ref->ToObject);
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









// ---------------------------------------------------------------------------------------------------------------------
// Decide operation Values

void UnimplementedValueFunction(const OperationType opType, Reference** refValue)
{
    *refValue = nullptr;
    LogIt(LogSeverityType::Sev1_Notify, "DecideOperationValue", MSG("unimplemented in case: %s", ToString(opType)));
}


void DecideValueDefine(TokenList& tokens, Reference** refValue)
{
    Reference* ref; 

    std::vector<String> arrayWords = { "array", "list", "collection" };
    if(TokenListContainsContent(tokens, arrayWords))
    {
        // do array stuff
        Token* name = NextTokenMatching(tokens, TokenType::Reference);
        Token* size = NextTokenMatching(tokens, TokenType::Integer);
        int* i = new int;
        *i = std::stoi(size->Content);

        *refValue = ReferenceFor(name->Content, ArrayClass, static_cast<void*>(i));
        return;
    }

    // treat as primitive
    Token* name = NextTokenMatching(tokens, TokenType::Reference);
    Token* value = NextTokenMatching(tokens, PrimitiveTokenTypes);

    if(name == nullptr){
        LogIt(LogSeverityType::Sev3_Critical, "DecideOperandsDefine", "cannot determine reference name");
        ReportCompileMsg(SystemMessageType::Exception, "cannot determine reference name");
        // TODO: should be critical error
        *refValue = NullReference(); 
        return;
    }
    
    if(value == nullptr)
    {
        ReportCompileMsg(SystemMessageType::Exception, "cannot determine reference value");
        ref = NullReference(name->Content);
    }
    else
    {
        ref = ReferenceFor(value, name->Content);
    }
    *refValue = ref;
}

void DecideValueAssign(TokenList& tokens, Reference** refValue)
{
    int pos = 0;
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, TokenType::Reference, pos));

    TokenList rightTokens = RightOfToken(tokens, tokens.at(pos));
    tokens = rightTokens;

    *refValue = arg1;
}

void DecideValueIsEqual(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::IsEqual, refValue);
}

void DecideValueIsLessThan(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::IsLessThan, refValue);
}

void DecideValueIsGreaterThan(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::IsGreaterThan, refValue);
}

void DecideValueAdd(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Add, refValue);
}

void DecideValueSubtract(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Subtract, refValue);
}

void DecideValueMultiply(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Multiply, refValue);
}

void DecideValueDivide(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Divide, refValue);
}

void DecideValueAnd(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::And, refValue);
}

void DecideValueOr(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Or, refValue);
}

void DecideValueNot(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Not, refValue);
}



void DecideValuePrint(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Print, refValue);
}

void DecideValueReturn(TokenList& tokens, Reference** refValue)
{
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes));
    *refValue = arg1;
}









// ---------------------------------------------------------------------------------------------------------------------
// Decide Operands
// should edit token list remove used tokens

void GetTwoOperands(TokenList& tokens, OperationsList& operands)
{
    int pos = 0;
 
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes, pos), c_operationReferenceName);
    Reference* arg2 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes, pos), c_operationReferenceName);

    AddReferenceReturnOperationTo(operands, arg1);
    AddReferenceReturnOperationTo(operands, arg2);
}


void DecideOperandsAdd(TokenList& tokens, OperationsList& operands) // EDIT
{
    GetTwoOperands(tokens, operands);
}

void DecideOperandsDefine(TokenList& tokens, OperationsList& operands)
{
    // no operands
}

void DecideOperandsPrint(TokenList& tokens, OperationsList& operands)
{
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes), c_operationReferenceName);
    AddReferenceReturnOperationTo(operands, arg1);
}

void DecideOperandsAssign(TokenList& tokens, OperationsList& operands)
{
    Operation* op2 = ParseLine(tokens);
    AddOperationTo(operands, op2);
}

void DecideOperandsIsEqual(TokenList& tokens, OperationsList& operands)
{

}

void DecideOperandsIsLessThan(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsIsGreaterThan(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsSubtract(TokenList& tokens, OperationsList& operands)
{
    GetTwoOperands(tokens, operands);
}

void DecideOperandsMultiply(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsDivide(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsAnd(TokenList& tokens, OperationsList& operands)
{
    GetTwoOperands(tokens, operands);
}

void DecideOperandsOr(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsNot(TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsReturn(TokenList& tokens, OperationsList& operands)
{
    // no operands
}



// ---------------------------------------------------------------------------------------------------------------------
// DefineMethod operation

Reference* OperationDefineMethod(Reference* ref)
{
    AddReferenceToCurrentScope(ref);
    return NullReference();
}

void DecideProbabilityDefineMethod(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(FindToken(tokens, "method") != nullptr && FindToken(tokens, ":") != nullptr)
    {
        OperationTypeProbability defineMethodType = { OperationType::DefineMethod, 10.0 };
        typeProbabilities.push_back(defineMethodType);
    }
}

void DecideOperandsDefineMethod(TokenList& tokens, OperationsList& operands)
{
    // no operands
}

void DecideValueDefineMethod(TokenList& tokens, Reference** refValue)
{
    // TODO: assumes first reference is method name
    int i=0;
    Token* t = NextTokenMatching(tokens, TokenType::Reference, i);
    String methodName = t->Content;
    
    Method* m = new Method;
    m->Parameters = ScopeConstructor(PROGRAM->GlobalScope);
    m->CodeBlock = BlockConstructor(m->Parameters);

    for(t = NextTokenMatching(tokens, TokenType::Reference, i); t != nullptr; t = NextTokenMatching(tokens, TokenType::Reference, i))
    {
        m->Parameters->ReferencesIndex.push_back(NullReference(t->Content));
    }
    
    Reference* ref = ReferenceFor(methodName, m);
    *refValue = ref;
}




// ---------------------------------------------------------------------------------------------------------------------
// Evaluate operation

Reference* OperationEvaluate(Reference* ref, std::vector<Reference*> parameters)
{
    // TODO: currently just assumes parameters are in order
    // ref should be a method reference
    Reference* result;
    for(size_t i=0; i<ref->ToMethod->Parameters->ReferencesIndex.size() && i<parameters.size()-1; i++)
    {
        Reference* paramRef = ref->ToMethod->Parameters->ReferencesIndex.at(i);
        ReassignReference(paramRef, parameters.at(i+1)->ToObject);   
    }
    result = DoBlock(ref->ToMethod->CodeBlock);
    AddReferenceToCurrentScope(result);
    
    for(Reference* param: ref->ToMethod->Parameters->ReferencesIndex)
    {
        AssignToNull(param);
    }

    // AddReferenceToCurrentScope(result);
    LogItDebug("finished evaluate", "OperationEvaluate");
    return result;
}

void DecideProbabilityEvaluate(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(FindToken(tokens, "evaluate") != nullptr)
    {
        OperationTypeProbability evaluateType = { OperationType::Evaluate, 10.0 };
        typeProbabilities.push_back(evaluateType);
    }
}

void DecideValueEvaluate(TokenList& tokens, Reference** refValue)
{
    UnimplementedValueFunction(OperationType::Evaluate, refValue);
}

void DecideOperandsEvaluate(TokenList& tokens, OperationsList& operands)
{
    int i = 0;
    for(Token* t = NextTokenMatching(tokens, TokenType::Reference, i); t != nullptr; t = NextTokenMatching(tokens, TokenType::Reference, i))
    {
        AddReferenceReturnOperationTo(operands, ReferenceFor(t));
    }
}