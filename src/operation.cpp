#include <iostream>

#include "operation.h"
#include "program.h"
#include "reference.h"




// ---------------------------------------------------------------------------------------------------------------------
// TODO:
// 1. Refactor code to put similar methods in the same section (i.e. DecideProbabilityX, OperationX, DecideOperandsX)
// 2. Implement unimplemented operations
// 3. Revamp the DecideOperandsX system





// ---------------------------------------------------------------------------------------------------------------------
// Creating operations

/// constructor for an operation of [type], with [operands], and value [value]. only operations
/// of type Ref::Operation should have non-nullptr value.
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

/// constructor for an operation of [type], with [operands], and value [value]. only operations
/// of type Ref::Operation should have non-nullptr value.
Operation* OperationConstructor(
    OperationType type, 
    Reference* value,
    OperationsList operands)
{
    return OperationConstructor(type, operands, value);
}

// constructor for a Method* object with [inheritedScope]
Method* MethodConstructor(Scope* inheritedScope)
{
    Method* m = new Method;
    m->Parameters = ScopeConstructor(inheritedScope);
    m->CodeBlock = BlockConstructor();
    m->Type = ReferableType::Method;

    return m;
}

/// adds a new Operation::Ref type operation for [ref] to [operands]
void AddRefOperationTo(OperationsList& operands, Reference* ref)
{
    operands.push_back(OperationConstructor(OperationType::Ref, ref));
}

/// adds an existing Operation [op] to [operands]
void AddOperationTo(OperationsList& operands, Operation* op)
{
    operands.push_back(op);
}




// ---------------------------------------------------------------------------------------------------------------------
// Atomic Operations

/// handles OperationType::Ref which returns a reference
Reference* OperationRef(Reference* ref)
{
    return ref;
}

/// handles OperationType::Assign which assigns a reference [lRef] to the Referable of [rRef]
/// returns a temporary reference to the assigned Referable
Reference* OperationAssign(Reference* lRef, Reference* rRef)
{
    ReassignReference(lRef, ObjectOf(rRef));
    return ReferenceFor(c_temporaryReferenceName, ObjectOf(lRef));
}

/// handles OperationType::Print which prints the string value of [ref]
/// returns a temporary reference to the printed Referable
Reference* OperationPrint(const Reference* ref)
{
    if(g_outputOn)
        std::cout << GetStringValue(*ObjectOf(ref)) << "\n";
    ProgramOutput.append(GetStringValue(*ObjectOf(ref)) + "\n");
    
    return ReferenceFor(c_temporaryReferenceName, ObjectOf(ref));
}

/// handles OperationType::Add which adds the objects of [lRef] and [rRef]
/// only supports adding objects of numeric type and Strings (by concatenation)
/// returns a temporary reference to the addition result, which is null on failure
Reference* OperationAdd(const Reference* lRef, const Reference* rRef)
{
    Reference* resultRef;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*ObjectOf(lRef), *ObjectOf(rRef));
        if(type == IntegerClass)
        {
            int value = GetIntValue(*ObjectOf(lRef)) + GetIntValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*ObjectOf(lRef)) + GetDecimalValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Add", "unimplemented");
            resultRef = NullReference();
        }
        
        return resultRef;
    }
    // allow for string addition by concatenation. this promotes all other objects to strings
    else if(IsString(lRef) || IsString(rRef))
    {
        String s = GetStringValue(*ObjectOf(lRef)) + GetStringValue(*ObjectOf(rRef));
        resultRef = ReferenceFor(c_temporaryReferenceName, s);

        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot add types %s and %s", 
        ObjectOf(lRef)->Class, 
        ObjectOf(rRef)->Class));

    return resultRef;
}

/// handles OperationType::And which returns the && of the boolean value for [lRef] and [rRef]
/// returns a temporary reference to the result
Reference* OperationAnd(const Reference* lRef, const Reference* rRef)
{
    bool b = GetBoolValue(*ObjectOf(lRef)) && GetBoolValue(*ObjectOf(rRef));
    return ReferenceFor(c_temporaryReferenceName, b);
}

/// handles OperationType::Define which adds a new reference to the current scope
/// returns the newly added [ref]
Reference* OperationDefine(Reference* ref)
{
    LogItDebug(Msg("added reference [%s] to scope", ref->Name), "OperationDefine");
    AddReferenceToCurrentScope(ref);

    return ref;
}

/// handles OperationType::Subtract which is only defined for numeric typed objects
/// returns a temporary reference to the resultant, null if failed
Reference* OperationSubtract(const Reference* lRef, const Reference* rRef)
{
    Reference* resultRef;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*ObjectOf(lRef), *ObjectOf(rRef));
        if(type == IntegerClass)
        {
            int value = GetIntValue(*ObjectOf(lRef)) - GetIntValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*ObjectOf(lRef)) - GetDecimalValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Subtract", "unimplemented");
            resultRef = NullReference();
        }
        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot subtract %s from %s", ObjectOf(rRef)->Class, ObjectOf(lRef)->Class));
    return resultRef;
}

/// handles OperationType::If 
/// returns a temporary reference to an object representing the evaluated if-expression
Reference* OperationIf(Reference* ref)
{
    return ReferenceFor(c_temporaryReferenceName, ObjectOf(ref));
}


/// handles OperationType::Multiply which is only defined for numeric typed objects
/// returns a temporary reference to the resultant, null if failed
Reference* OperationMultiply(const Reference* lRef, const Reference* rRef)
{
    Reference* resultRef;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*ObjectOf(lRef), *ObjectOf(rRef));
        if(type == IntegerClass)
        {
            int value = GetIntValue(*ObjectOf(lRef)) * GetIntValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*ObjectOf(lRef)) * GetDecimalValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Multiply", "unimplemented");
            resultRef = NullReference();
        }
        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot multiply %s and %s", ObjectOf(lRef)->Class, ObjectOf(rRef)->Class));
    return resultRef;
}

/// handles OperationType::Divide which is only defined for numeric typed objects
/// returns a temporary reference to the resultant, null if failed
Reference* OperationDivide(const Reference* lRef, const Reference* rRef)
{
    Reference* resultRef;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {  
        ObjectClass type = GetPrecedenceClass(*ObjectOf(lRef), *ObjectOf(rRef));
        if(type == IntegerClass)
        {
            int value = GetIntValue(*ObjectOf(lRef)) / GetIntValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else if(type == DecimalClass)
        {
            double value = GetDecimalValue(*ObjectOf(lRef)) / GetDecimalValue(*ObjectOf(rRef));
            resultRef = ReferenceFor(c_temporaryReferenceName, value);
        }
        else
        {
            LogIt(LogSeverityType::Sev1_Notify, "Divide", "unimplemented");
            resultRef = NullReference();
        }
        return resultRef;
    }

    resultRef = NullReference();
    ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot divide %s by %s", ObjectOf(lRef)->Class, ObjectOf(rRef)->Class));
    return resultRef;
}

/// handles OperationType::Return to exit a method
/// returns a persistant reference to the return value
Reference* OperationReturn(Reference* returnRef)
{
    // TODO: can only return objects for now
    return ReferenceFor(c_returnReferenceName, ObjectOf(returnRef));
}


// ---------------------------------------------------------------------------------------------------------------------
// Decide Probabilities

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityAdd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> addKeyWords = { "add", "plus", "+", "adding" };
    if(TokenListContainsContent(tokens, addKeyWords))
    {
        OperationTypeProbability addType = { OperationType::Add, 4.0 };
        typeProbabilities.push_back(addType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityDefine(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> defineKeyWords = { "define", "let", "make", "declare" };
    if(TokenListContainsContent(tokens, defineKeyWords))
    {
        OperationTypeProbability defineType = { OperationType::Define, 4.0};
        typeProbabilities.push_back(defineType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityPrint(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> printKeyWords = { "print", "display", "show", "output" };
    if(TokenListContainsContent(tokens, printKeyWords))
    {
        OperationTypeProbability printType = { OperationType::Print, 4.0 };
        typeProbabilities.push_back(printType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityAssign(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(Token* pos = FindToken(tokens, "="); pos != nullptr)
    {
        OperationTypeProbability assignType = { OperationType::Assign, 6 };
        typeProbabilities.push_back(assignType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityRef(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    OperationTypeProbability returnType = { OperationType::Ref, 1 };
    typeProbabilities.push_back(returnType);
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityIsEqual(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{

}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityIsLessThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityIsGreaterThan(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilitySubtract(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> subKeyWords = { "sub", "subtract", "minus", "-", "subtracting" };
    if(TokenListContainsContent(tokens, subKeyWords))
    {
        OperationTypeProbability subType = { OperationType::Subtract, 4.0 };
        typeProbabilities.push_back(subType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityMultiply(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityDivide(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityAnd(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    std::vector<String> andKeyWords = { "and", "&&", "together", "with" };
    if(TokenListContainsContent(tokens, andKeyWords))
    {
        OperationTypeProbability andType = { OperationType::And, 4.0 };
        typeProbabilities.push_back(andType);
    }
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityOr(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityNot(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    
}


// ---------------------------------------------------------------------------------------------------------------------
// Decide operation reference value

/// handles OperationType::Ref by assigning the operation.Value to the approriate reference
void DecideValueRef(TokenList& tokens, Reference** refValue)
{
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes), c_operationReferenceName);
    *refValue = arg1;
}


// ---------------------------------------------------------------------------------------------------------------------
// Decide Operands
// should edit token list remove used tokens

/// gets references for the next two tokens which refer to Objects
void GetTwoOperands(TokenList& tokens, OperationsList& operands)
{
    int pos = 0;
 
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes, pos), c_operationReferenceName);
    Reference* arg2 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes, pos), c_operationReferenceName);

    AddRefOperationTo(operands, arg1);
    AddRefOperationTo(operands, arg2);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsAdd(TokenList& tokens, OperationsList& operands) // EDIT
{
    GetTwoOperands(tokens, operands);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsDefine(TokenList& tokens, OperationsList& operands)
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

        AddRefOperationTo(operands, ReferenceFor(name->Content, ArrayClass, static_cast<void*>(i)));
        return;
    }

    // treat as primitive
    Token* name = NextTokenMatching(tokens, TokenType::Reference);
    Token* value = NextTokenMatching(tokens, PrimitiveTokenTypes);

    if(name == nullptr){
        LogIt(LogSeverityType::Sev3_Critical, "DecideOperandsDefine", "cannot determine reference name");
        ReportCompileMsg(SystemMessageType::Exception, "cannot determine reference name");
        // TODO: should be critical error
        AddRefOperationTo(operands, NullReference()); 
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
    AddRefOperationTo(operands, ref);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsPrint(TokenList& tokens, OperationsList& operands)
{
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, ObjectTokenTypes), c_operationReferenceName);
    AddRefOperationTo(operands, arg1);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsAssign(TokenList& tokens, OperationsList& operands)
{

    int pos = 0;
    Reference* arg1 = ReferenceFor(NextTokenMatching(tokens, TokenType::Reference, pos));
    AddRefOperationTo(operands, arg1);

    TokenList rightTokens = RightOfToken(tokens, tokens.at(pos));
    tokens = rightTokens;

    Operation* op2 = ParseLine(tokens);
    AddOperationTo(operands, op2);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsIsEqual(TokenList& tokens, OperationsList& operands)
{

}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsIsLessThan(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsIsGreaterThan(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsSubtract(TokenList& tokens, OperationsList& operands)
{
    GetTwoOperands(tokens, operands);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsMultiply(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsDivide(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsAnd(TokenList& tokens, OperationsList& operands)
{
    GetTwoOperands(tokens, operands);
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsOr(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsNot(TokenList& tokens, OperationsList& operands)
{
    
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsRef(TokenList& tokens, OperationsList& operands)
{
    // no operands
}



// ---------------------------------------------------------------------------------------------------------------------
// OperationType::DefineMethod

/// handle the operation which adds a Method [ref] to the scope
/// returns a 
Reference* OperationDefineMethod(Reference* ref)
{
    AddReferenceToCurrentScope(ref);
    return NullReference();
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityDefineMethod(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(FindToken(tokens, "method") != nullptr && FindToken(tokens, ":") != nullptr)
    {
        OperationTypeProbability defineMethodType = { OperationType::DefineMethod, 10.0 };
        typeProbabilities.push_back(defineMethodType);
    }
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsDefineMethod(TokenList& tokens, OperationsList& operands)
{
    // TODO: assumes first reference is method name
    int i=0;
    Token* t = NextTokenMatching(tokens, TokenType::Reference, i);
    String methodName = t->Content;
    
    Method* m = MethodConstructor(PROGRAM->GlobalScope);

    for(t = NextTokenMatching(tokens, TokenType::Reference, i); t != nullptr; t = NextTokenMatching(tokens, TokenType::Reference, i))
    {
        m->Parameters->ReferencesIndex.push_back(NullReference(t->Content));
    }
    
    Reference* ref = ReferenceFor(methodName, m);
    AddRefOperationTo(operands, ref);
}


// ---------------------------------------------------------------------------------------------------------------------
// Tuple operation
Reference* OperationTuple(const std::vector<Reference*>& components)
{
    LogItDebug("called", "OperationTuple");
    Reference* tupleRef = ReferenceFor(c_temporaryReferenceName, TupleClass, nullptr);
    ObjectOf(tupleRef)->Attributes.reserve(components.size());

    for(auto ref: components)
    {
        auto copyRef = NullReference(ref->Name);
        ReassignReference(copyRef, ref->To);
        ObjectOf(tupleRef)->Attributes.push_back(copyRef);
    }

    return tupleRef;
}


// ---------------------------------------------------------------------------------------------------------------------
// Evaluate operation

std::vector<Reference*> ResolveParamters(Reference* ref, std::vector<Reference*>& parameters)
{
    if(parameters.size()<=1)
        return {};

    auto obj = ObjectOf(parameters.at(1));
    if(obj->Class == TupleClass)
    {
        std::vector<Reference*> params;
        for(auto ref: obj->Attributes)
        {
            params.push_back(ref);
        }
        return params;
    }
    else
    {
        return { parameters.at(1) };
    }
    
}

/// handles OperationType::Evaluate which takes in a Method reference [ref] and [parameters], the first
/// of which is a reference to the method, and evaluates the method on these parameters
/// returns a persistant reference to the returned result if a return statement was called
/// or a temporary reference if no return statement was called
Reference* OperationEvaluate(Reference* ref, std::vector<Reference*>& parameters)
{
    // TODO: currently just assumes parameters are in order
    // ref should be a method reference
    LogItDebug(Msg("evaluating method %s", ref->Name), "OperationEvaluate");

    std::vector<Referable*> originalParams;
    std::vector<Reference*> params = ResolveParamters(ref, parameters);

    Reference* result;
    for(size_t i=0; i<MethodOf(ref)->Parameters->ReferencesIndex.size() && i<params.size(); i++)
    {
        Reference* paramRef = MethodOf(ref)->Parameters->ReferencesIndex.at(i);
        originalParams.push_back(paramRef->To);
        auto inputRef = params.at(i);
        ReassignReference(paramRef, inputRef->To);   
    }

    LogDiagnostics(MethodOf(ref)->CodeBlock, "method codeblock");

    EnterScope(MethodOf(ref)->Parameters);
    result = DoBlock(MethodOf(ref)->CodeBlock);
    ExitScope();
    LogDiagnostics(result, "evaluate result");

    AddReferenceToCurrentScope(result);
    
    for(size_t i=0; i<originalParams.size(); i++)
    {
        ReassignReference(MethodOf(ref)->Parameters->ReferencesIndex.at(i), originalParams.at(i));
    }

    LogItDebug("finished evaluate", "OperationEvaluate");
    return result;
}

/// computes the probability that a given line convered into [tokens] is this atomic operation
/// adds a new OperationTypeProbability to [typeProbabilities]
void DecideProbabilityEvaluate(PossibleOperationsList& typeProbabilities, const TokenList& tokens)
{
    if(FindToken(tokens, "(") != nullptr)
    {
        OperationTypeProbability evaluateType = { OperationType::Evaluate, 3.0 };
        typeProbabilities.push_back(evaluateType);
    }
}

/// given [tokens], queries the proper operands required for this operation and adds them to [operands]
void DecideOperandsEvaluate(TokenList& tokens, OperationsList& operands)
{
    // function name is first parameter
    int i = 0;
    for(Token* t = NextTokenMatching(tokens, TokenType::Reference, i); t != nullptr; t = NextTokenMatching(tokens, TokenType::Reference, i))
    {
        AddRefOperationTo(operands, ReferenceFor(t));
    }
}
