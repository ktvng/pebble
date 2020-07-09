#include <iostream>

#include "operation.h"
#include "program.h"
#include "reference.h"
#include "execute.h"
#include "main.h"
#include "token.h"
#include "object.h"
#include "scope.h"
#include "diagnostics.h"


// ---------------------------------------------------------------------------------------------------------------------
// TODO:
// 1. Refactor code to put similar methods in the same section (i.e. DecideProbabilityX, OperationX, DecideOperandsX)
// 2. Implement unimplemented operations
// 3. Revamp the DecideOperandsX system


// ---------------------------------------------------------------------------------------------------------------------
// Operations
OperationEvaluator OperationEvaluators[] = 
{
    OperationAssign,
    OperationIsEqual,
    OperationIsNotEqual,
    OperationIsLessThan,
    OperationIsGreaterThan,
    OperationIsLessThanOrEqualTo,
    OperationIsGreaterThanOrEqualTo,

    OperationAdd,
    OperationSubtract,
    OperationMultiply,
    OperationDivide,

    OperationAnd,
    OperationOr,
    OperationNot,
    OperationEvaluate,
    OperationPrint,

    OperationRef,
    OperationDefineMethod,
    OperationReturn,

    OperationIf,
    OperationWhile,
    OperationEndLabel,
    OperationTuple,

    OperationNew,
    OperationScopeResolution,
    OperationClass,


};


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

Reference* ResolveStub(Reference* ref) 
{
    if(!IsReferenceStub(ref))
        return ref;

    if(ref->Name == "that" || ref->Name == "it")
    {
        if(PROGRAM->That == nullptr)
        {
            ReportRuntimeMsg(SystemMessageType::Exception, "no previous result to use with 'that'/'it' keyword");
            return NullReference(ref->Name);
        }
        return ReferenceFor(c_temporaryReferenceName, PROGRAM->That->To);
    }
    
    auto lookupRef = ReferenceFor(ref->Name);
    if(lookupRef == nullptr)
        return NullReference(ref->Name);
    else
        return lookupRef;
}

inline Reference* NthOf(std::vector<Reference*>& operands, int n)
{
    if(operands.size() >= static_cast<size_t>(n))
        return ResolveStub(operands.at(n-1));
    return nullptr;
}

inline Reference* FirstOf(std::vector<Reference*>& operands)
{
    return NthOf(operands, 1);
}

inline Reference* SecondOf(std::vector<Reference*>& operands)
{
    return NthOf(operands, 2);
}

bool ReferencesAreEqual(Reference* lhs, Reference* rhs)
{
    return lhs->To == rhs->To;
}

/// TODO: allow intra primitive comparisions
Reference* OperationIsEqual(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lhs = FirstOf(operands);
    Reference* rhs = SecondOf(operands);
    bool areEqual = ReferencesAreEqual(lhs, rhs);
    
    return ReferenceFor(c_temporaryReferenceName, areEqual);
}

Reference* OperationIsNotEqual(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lhs = FirstOf(operands);
    Reference* rhs = SecondOf(operands);
    bool areEqual = ReferencesAreEqual(lhs, rhs);
    
    return ReferenceFor(c_temporaryReferenceName, !areEqual);
}

Reference* OperationIsLessThan(Reference* value, std::vector<Reference*>& operands)
{
    auto lRef = FirstOf(operands);
    auto rRef = SecondOf(operands);
    bool comparisonIs = false;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {
        comparisonIs = GetDecimalValue(*ObjectOf(lRef)) < GetDecimalValue(*ObjectOf(rRef));
    }

    return ReferenceFor(c_temporaryReferenceName, comparisonIs);
}

Reference* OperationIsGreaterThan(Reference* value, std::vector<Reference*>& operands)
{
    auto lRef = FirstOf(operands);
    auto rRef = SecondOf(operands);
    bool comparisonIs = false;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {
        comparisonIs = GetDecimalValue(*ObjectOf(lRef)) > GetDecimalValue(*ObjectOf(rRef));
    }

    return ReferenceFor(c_temporaryReferenceName, comparisonIs);
}

Reference* OperationIsLessThanOrEqualTo(Reference* value, std::vector<Reference*>& operands)
{
    auto lRef = FirstOf(operands);
    auto rRef = SecondOf(operands);
    bool comparisonIs = false;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {
        comparisonIs = GetDecimalValue(*ObjectOf(lRef)) <= GetDecimalValue(*ObjectOf(rRef));
    }

    return ReferenceFor(c_temporaryReferenceName, comparisonIs);
}

Reference* OperationIsGreaterThanOrEqualTo(Reference* value, std::vector<Reference*>& operands)
{
    auto lRef = FirstOf(operands);
    auto rRef = SecondOf(operands);
    bool comparisonIs = false;

    if(IsNumeric(lRef) && IsNumeric(rRef))
    {
        comparisonIs = GetDecimalValue(*ObjectOf(lRef)) >= GetDecimalValue(*ObjectOf(rRef));
    }
    
    return ReferenceFor(c_temporaryReferenceName, comparisonIs);
}


Reference* OperationOr(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = FirstOf(operands);
    Reference* rRef = SecondOf(operands);

    bool b = GetBoolValue(*ObjectOf(lRef)) || GetBoolValue(*ObjectOf(rRef));
    return ReferenceFor(c_temporaryReferenceName, b);
}

Reference* OperationNot(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = FirstOf(operands);

    bool b = !GetBoolValue(*ObjectOf(ref));
    return ReferenceFor(c_temporaryReferenceName, b);
}

Reference* OperationEndLabel(Reference* value, std::vector<Reference*>& operands)
{
    LogIt(LogSeverityType::Sev1_Notify, "OperationEndLabel", "unimplemented");
    return NullReference();
}


// ---------------------------------------------------------------------------------------------------------------------
// Atomic Operations

/// handles OperationType::Ref which returns a reference
Reference* OperationRef(Reference* value, std::vector<Reference*>& operands)
{
    return value;
}

/// handles OperationType::Assign which assigns a reference [lRef] to the Referable of [rRef]
/// returns a temporary reference to the assigned Referable
Reference* OperationAssign(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = FirstOf(operands);
    Reference* rRef = SecondOf(operands);

    if(IsNullReference(rRef))
    {
        ReportRuntimeMsg(SystemMessageType::Warning, Msg("cannot assign %s to Nothing", lRef));
        return ReferenceFor(c_temporaryReferenceName, lRef->To);
    }

    ReassignReference(lRef, rRef->To);
    return ReferenceFor(c_temporaryReferenceName, lRef->To);
}

/// handles OperationType::Print which prints the string value of [ref]
/// returns a temporary reference to the printed Referable
Reference* OperationPrint(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = FirstOf(operands);

    if(g_outputOn)
        std::cout << GetStringValue(*ObjectOf(ref)) << "\n";
    ProgramOutput.append(GetStringValue(*ObjectOf(ref)) + "\n");
    
    return ReferenceFor(c_temporaryReferenceName, ObjectOf(ref));
}

/// handles OperationType::Add which adds the objects of [lRef] and [rRef]
/// only supports adding objects of numeric type and Strings (by concatenation)
/// returns a temporary reference to the addition result, which is null on failure
Reference* OperationAdd(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = FirstOf(operands);
    Reference* rRef = SecondOf(operands);
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
Reference* OperationAnd(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = FirstOf(operands);
    Reference* rRef = SecondOf(operands);

    bool b = GetBoolValue(*ObjectOf(lRef)) && GetBoolValue(*ObjectOf(rRef));
    return ReferenceFor(c_temporaryReferenceName, b);
}

/// handles OperationType::Define which adds a new reference to the current scope
/// returns the newly added [ref]
Reference* OperationDefine(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = FirstOf(operands);
    Reference* containerRef = SecondOf(operands);

    if(containerRef != nullptr)
    {
        EnterScope(ObjectOf(containerRef)->Attributes);
        {
            AddReferenceToCurrentScope(ref);
        }
        ExitScope();
    }
    else
    {
        AddReferenceToCurrentScope(ref);
    }

    LogItDebug(Msg("added reference [%s] to scope", ref->Name), "OperationDefine");

    return ref;
}

/// handles OperationType::Subtract which is only defined for numeric typed objects
/// returns a temporary reference to the resultant, null if failed
Reference* OperationSubtract(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = FirstOf(operands);
    Reference* rRef = SecondOf(operands);
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
Reference* OperationIf(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = FirstOf(operands);
    return ReferenceFor(c_temporaryReferenceName, ObjectOf(ref));
}

Reference* OperationWhile(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = FirstOf(operands);
    return ReferenceFor(c_temporaryReferenceName, ObjectOf(ref));
}


/// handles OperationType::Multiply which is only defined for numeric typed objects
/// returns a temporary reference to the resultant, null if failed
Reference* OperationMultiply(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = FirstOf(operands);
    Reference* rRef = SecondOf(operands);
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

// ---------------------------------------------------------------------------------------------------------------------
// DivideOperation

/// handles OperationType::Divide which is only defined for numeric typed objects
/// returns a temporary reference to the resultant, null if failed
Reference* OperationDivide(Reference* value, std::vector<Reference*>& operands)
{
    Reference* lRef = FirstOf(operands);
    Reference* rRef = SecondOf(operands);
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
Reference* OperationReturn(Reference* value, std::vector<Reference*>& operands)
{
    Reference* returnRef = FirstOf(operands);
    return ReferenceFor(c_returnReferenceName, returnRef->To);
}





// ---------------------------------------------------------------------------------------------------------------------
// OperationType::DefineMethod

/// handle the operation which adds a Method [ref] to the scope
Reference* OperationDefineMethod(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = FirstOf(operands);
    if(operands.size() == 2)
    {
        Reference* containingRef = SecondOf(operands);
        EnterScope(ObjectOf(containingRef)->Attributes);
        {
            AddReferenceToCurrentScope(ref);
        }
        ExitScope();
    }
    else
    {
        AddReferenceToCurrentScope(ref);
    }
    
    return NullReference();
}




// ---------------------------------------------------------------------------------------------------------------------
// Tuple operation
Reference* OperationTuple(Reference* value, std::vector<Reference*>& operands)
{
    LogItDebug("called", "OperationTuple");
    Reference* tupleRef = ReferenceFor(c_temporaryReferenceName, TupleClass, nullptr);

    for(auto ref: operands)
    {
        EnterScope(ObjectOf(tupleRef)->Attributes);
        {
            auto copyRef = NullReference(ref->Name);
            ReassignReference(copyRef, ref->To);
        }
        ExitScope();

    }

    return tupleRef;
}


// ---------------------------------------------------------------------------------------------------------------------
// Evaluate operation

std::vector<Reference*> ResolveParamters(Reference* ref)
{
    if(IsNullReference(ref))
        return {};

    auto obj = ObjectOf(ref);
    if(obj != nullptr && obj->Class == TupleClass)
    {
        std::vector<Reference*> params;
        for(auto ref: obj->Attributes->ReferencesIndex)
        {
            params.push_back(ref);
        }
        return params;
    }
    else
    {
        return { ref };
    }
    
}

/// methods are called by a caller, and if no caller is specified, CurrentScope takes on the role of 
/// caller. the order of operands should be [caller] [methodname] [arguments]
/// handles OperationType::Evaluate which takes in a Method reference [ref] and [parameters], the first
/// of which is a reference to the method, and evaluates the method on these parameters
/// returns a persistant reference to the returned result if a return statement was called
/// or a temporary reference if no return statement was called
Reference* OperationEvaluate(Reference* value, std::vector<Reference*>& operands)
{
    auto explictCallerRef = FirstOf(operands);
    Reference* caller;

    // no explicit caller is given
    if(IsNullReference(explictCallerRef))
    {
        // look for native 'caller' Referene
        caller = ReferenceFor("caller");
        if(caller == nullptr)
            caller = NullReference();
    }
    else
    {
        caller = explictCallerRef;
    }
    // determine caller scope
    Scope* callerScope;
    if(IsNullReference(caller))
    {
        /// TODO: change this b/c this doesnt work well
        callerScope = CurrentScope();
    }
    else
    {
        callerScope = ObjectOf(caller)->Attributes;
    }
    // resolve method call references inside caller scope
    Reference* method;
    EnterScope(callerScope);
    {
        method = SecondOf(operands);
    }
    ExitScope();
    
    if(IsNullReference(method))
    {
        EnterScope(CurrentScope());
        {
            method = SecondOf(operands);
        }
        ExitScope();
    }

    if(IsNullReference(method))
    {
        ReportRuntimeMsg(SystemMessageType::Exception, Msg("cannot resolve %s into a method", method->Name));
        return NullReference();
    }

    Reference* params = NthOf(operands, 3);

    
    // resolve param refs from params
    std::vector<Reference*> givenParamsList = ResolveParamters(params);
    auto givenNum = givenParamsList.size();
    auto requiredNum = ObjectOf(method)->Action->ParameterNames.size();
    if(givenNum != requiredNum)
    {
        ReportRuntimeMsg(SystemMessageType::Exception, Msg("wrong number of parameters: given %i, but expected %i", givenNum, requiredNum));
        return NullReference();
    }

    auto methodParamNames = ObjectOf(method)->Action->ParameterNames;

    // add parameters to method scope
    auto methodScope = ScopeConstructor(callerScope);
    EnterScope(methodScope);
    {
        for(size_t i=0; i<methodParamNames.size(); i++)
        {
            ReferenceFor(methodParamNames[i], givenParamsList[i]->To);
        }
        ReferenceFor("caller", caller->To);
    }
    ExitScope();

    Reference* result;
    auto methodBlock = ObjectOf(method)->Action->CodeBlock;
    result = DoBlock(methodBlock, methodScope);
    AddReferenceToCurrentScope(result);

    LogDiagnostics(result, "evaluate result");

    LogItDebug("finished evaluate", "OperationEvaluate");
    return result;
}



// ---------------------------------------------------------------------------------------------------------------------
// New Operation

/// creates a new object and copies all attributes
Reference* OperationNew(Reference* value, std::vector<Reference*>& operands)
{
    Reference* ref = FirstOf(operands);
    
    for(auto ref: PROGRAM->GlobalScope->ReferencesIndex)
        LogDiagnostics(ref, "dubaduba");

    LogDiagnostics(ref);
    if(IsNullReference(ref))
    {
        ReportRuntimeMsg(SystemMessageType::Exception, Msg("%s is Nothing, it must be defined before use", ref->Name));
        return NullReference();
    }

    Reference* returnRef = ReferenceFor(c_temporaryReferenceName, ObjectOf(ref)->Class, ObjectOf(ref)->Value);
    for(auto attributeRef: ObjectOf(ref)->Attributes->ReferencesIndex)
    {
        EnterScope(ObjectOf(returnRef)->Attributes);
        {
            ReferenceFor(attributeRef->Name, attributeRef->To);
        }
        ExitScope();
    }
    ObjectOf(returnRef)->Attributes->InheritedScope = ObjectOf(ref)->Attributes->InheritedScope;

    return returnRef;
}

// ---------------------------------------------------------------------------------------------------------------------
// Scope Resolution Operation

/// resolves one link in a scope chain
Reference* OperationScopeResolution(Reference* value, std::vector<Reference*>& operands)
{
    Reference* inContext = FirstOf(operands);
    Reference* lookFor = SecondOf(operands);
    Reference* returnRef;

    /// if there is no scope chain, return the first argument
    if(lookFor == nullptr)
        return inContext;

    /// if the context we are inside is Nothing, raise error and continue propgating Nothing
    if(IsNullReference(inContext))
    {
        ReportRuntimeMsg(SystemMessageType::Exception, Msg("%s is Nothing and has no attribute %s", inContext->Name, lookFor->Name));
        return NullReference(lookFor->Name);
    }
    
    EnterScope(ObjectOf(inContext)->Attributes);
    {
        returnRef = ReferenceFor(lookFor->Name);
        if(returnRef == nullptr)
        {
            returnRef = NullReference(lookFor->Name);
        }
    }
    ExitScope();

    return returnRef;
}


// ---------------------------------------------------------------------------------------------------------------------
// Class Operation

Reference* OperationClass(Reference* value, std::vector<Reference*>& operands)
{
    /// TODO: clean up and handle inheritance
    Reference* klass = FirstOf(operands);
    Reference* tempRefToNewObj = ReferenceFor(c_temporaryReferenceName, klass->Name, nullptr);
    ReassignReference(klass, tempRefToNewObj->To);
    ObjectOf(klass)->Attributes->InheritedScope = PROGRAM->GlobalScope;
    Dereference(tempRefToNewObj);

    return klass;
}