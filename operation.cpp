#include <iostream>

#include "operation.h"
#include "program.h"

void AddReferenceToScope(Reference* ref, Scope* scope)
{
    scope->ReferencesIndex.push_back(ref);
}


// Creating operations

/// creates a Operation with OperationType::Return to return [ref]
Operation* CreateReturnOperation(Reference* ref)
{
    Operation* op = new Operation;
    op->Type = OperationType::Return;
    op->Value = ref;

    return op;
}

/// adds a new return Operation for [ref] to [operands]
void AddReferenceReturnOperationTo(OperationsList& operands, Reference* ref)
{
    operands.push_back(CreateReturnOperation(ref));
}

/// adds an existing Operation [op] to [operands]
void AddOperationTo(OperationsList& operands, Operation* op)
{
    operands.push_back(op);
}










// Atomic Operations
///
Reference* OperationAssign(Reference* lRef, Reference* rRef)
{
    lRef->ToObject = rRef->ToObject;
    return CreateReference(c_returnReferenceName, lRef->ToObject);
}

/// 
Reference* OperationPrint(Reference* ref)
{
    std::cout << GetStringValue(*ref->ToObject) << "\n";
    return CreateReference(c_returnReferenceName, ref->ToObject);
}

///
Reference* OperationAdd(const Reference* lRef, const Reference* rRef)
{
    Reference* resultRef;

    if(IsNumeric(*lRef) && IsNumeric(*rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*lRef->ToObject, *rRef->ToObject);
        if(type == IntegerClass)
        {
            int value = GetIntValue(*lRef->ToObject) + GetIntValue(*rRef->ToObject);
            resultRef = CreateReferenceToNewObject(c_returnReferenceName, IntegerClass, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*lRef->ToObject) + GetDecimalValue(*rRef->ToObject);
            resultRef = CreateReferenceToNewObject(c_returnReferenceName, DecimalClass, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Add", "unimplemented");
            resultRef = CreateNullReference();
        }
        return resultRef;
    }

    resultRef = CreateNullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, MSG("cannot add types %s and %s", lRef->ToObject->Class, rRef->ToObject->Class));
    return resultRef;
}

///
Reference* OperationAnd(const Reference* lRef, const Reference* rRef)
{
    bool b = GetBoolValue(*lRef->ToObject) && GetBoolValue(*rRef->ToObject);
    return CreateReferenceToNewObject(c_returnReferenceName, BooleanClass, b);
}

///
Reference* OperationDefine(Reference* ref)
{
    return CreateNullReference();
}




// Decide Probabilities
void DecideProbabilityAdd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> addKeyWords = { "add", "plus", "+", "adding", "declare" };
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
        OperationTypeProbability assignType = { OperationType::Assign, 10.0/pos->Position };
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
    
}

void DecideProbabilityMultiply(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityDivide(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

void DecideProbabilityAnd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
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






// Decide Operands
// should edit token list remove used tokens
void DecideOperandsAdd(Scope* scope, TokenList& tokens, OperationsList& operands) // EDIT
{
    int pos = 0;
 
    Reference* arg1 = DecideReferenceOf( scope, NextTokenMatching(tokens, ObjectTokenTypes, pos) );
    Reference* arg2 = DecideReferenceOf( scope, NextTokenMatching(tokens, ObjectTokenTypes, pos) );

    AddReferenceReturnOperationTo(operands, arg1);
    AddReferenceReturnOperationTo(operands, arg2);
}

void DecideOperandsDefine(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    Reference* ref; 
    Token* name = NextTokenMatching(tokens, TokenType::Reference);
    Token* value = NextTokenMatching(tokens, PrimitiveTokenTypes);
    if(name == nullptr){
        LogIt(LogSeverityType::Sev3_Critical, "DecideOperandsDefine", "cannot determine reference name");
        ReportCompileMsg(SystemMessageType::Exception, "cannot determine reference name");
        return;
    }
    
    if(value == nullptr)
    {
        ReportCompileMsg(SystemMessageType::Exception, "cannot determine reference value");
        ref = CreateNullReference(); 
    }
    else
    {
        ref = CreateReferenceToNewObject(name, value);
    }
    LogDiagnostics(ref, "added a reference", "DecideOperandsDefine");
    AddReferenceToScope(ref, scope);
}

void DecideOperandsPrint(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    Reference* arg1 = DecideReferenceOf(scope, NextTokenMatching(tokens, ObjectTokenTypes));
    AddReferenceReturnOperationTo(operands, arg1);
}

void DecideOperandsAssign(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    int pos = 0;
    Reference* arg1 = DecideReferenceOf(scope, NextTokenMatching(tokens, TokenType::Reference, pos));

    TokenList rightTokens = RightOfToken(tokens, tokens.at(pos));
    Operation* op2 = ParseLine(scope, rightTokens);

    AddReferenceReturnOperationTo(operands, arg1);
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
    
}

void DecideOperandsMultiply(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsDivide(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
}

void DecideOperandsAnd(Scope* scope, TokenList& tokens, OperationsList& operands)
{
    
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
    Reference* arg1 = DecideReferenceOf(scope, NextTokenMatching(tokens, ObjectTokenTypes));
    AddReferenceReturnOperationTo(operands, arg1);
}